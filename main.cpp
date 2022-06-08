#include <zmq.hpp>
#include "src/network/ServerMessaging.h"
#include <iostream>
#include <boost/program_options.hpp>

namespace po=boost::program_options;

double compute_number_of_doubles(unsigned int n, unsigned int m, unsigned int dim){
    // The number of bins is given by ceil(log2(n/m)+2)
    // The algorithm will also store q, which is another bin.
    // As a consequence it will take one more bin.
    // Each bin stores m points, each of size dim.
    return 1.0*(ceil(log2(1.0*n/m))+1) * m * dim;
}

struct distance_type {
    distance_type(std::string const& val):
            value(val)
    {
        if(val== "euclidean"){
            dist = Distance::Euclidean;
        }
        if(val == "cosine"){
            dist = Distance::Cosine;
        }
        if(val == "correlation"){
            dist = Distance::Correlation;
        }

    }
    std::string value;
    Distance dist;
};

void validate(boost::any& v,
              std::vector<std::string> const& values,
              distance_type* /* target_type */,
              int)
{
    using namespace boost::program_options;

    // Make sure no previous assignment to 'v' was made.
    validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    std::string const& s = validators::get_single_string(values);

    if (s == "euclidean" || s == "cosine" || s == "correlation") {
        v = boost::any(distance_type(s));
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}

int main(int argc, char **argv) {
    unsigned int input_m;
    unsigned int input_n;
    unsigned int input_dimension;
    unsigned int port;
    double allowable_ram;
    distance_type dist("euclidean");

    po::options_description desc("Launches a server to which it is possible to send a stream of points, maintaining the coreset representation described  in  Ackermann, Marcel R., et al. \"Streamkm++ a clustering algorithm for data streams.\" Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1."
                                 "\nShortly, the idea is to use a tree to partition space around representative points, which serve as proxy-centroids."
                                 "When one wishes to perform k-means, an approximate solution can be obtained using the representative points, thereby greatly reducing computational costs."
                                 "\nThe server allows to recover at any time either the representatives, or the centroids for a specified number of clusters (performing Kmeans++ in the latter case)."
                                 " Server can also be stopped upon client request, and they communicate via a inter-process communication."
                                 "\nAllowed options are");
    desc.add_options()
            ("help", "display this help message")
            ("n_samples", po::value<unsigned int>(&input_n)->default_value(200000),
                    "Approximate number of points to pass to the clustering. Does not need to be exact,"
                    "serves to estimate how many buckets to use when performing streamed clustering. Because the relationship"
                    "is logarithmic, the order of magnitude is sufficient.")
            ("n_representatives", po::value<unsigned int>(&input_m)->default_value(1800),
                    "Number of representative points. In other words, how many points will be kept to "
                    "perform clustering operations. A good approximation is around 200k, k the number of centroids you expect.")
            ("dim", po::value<unsigned int>(&input_dimension)->default_value(70000),
                    "Expected dimensionality of points. This is just an estimation, so as to estimate if RAM requirements can be fulfilled and in no way will constrain the server.")
            ("ram_limit", po::value<double>(&allowable_ram)->default_value(8.0),
                    "Allowable RAM memory, in giga bytes (GB) to be used over the course of the algorithm."
                    " Note that we expect 1KB = 1024B for example. If the algorithm, "
                    "because of the number of samples and number of representatives should fail to meet the RAM limit, "
                    "the program will raise an exception, indicating a sufficient number of representatives that would fulfill RAM requirement.")
            ("distance", po::value<distance_type>(&dist), "Distance to use in the coreset and in kmeans. Can be correlation, cosine or euclidean.")
            ("process_id", po::value<unsigned int>(&port)->default_value(1), "The process ID to use to communicate. Exactly one client may communicate with the server at a time.");
            ;

    po::variables_map vm;
    try{
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch(boost::program_options::validation_error &e){
        std::cout << "Invalid value for the distance. Valid values are euclidean, cosine or correlation" << std::endl;
        return 1;
    } catch(std::exception &e){
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if(allowable_ram <= 0 ){
        std::cout << "Please provide a positive number for the RAM." << std::endl;
        return 1;
    }

    if(input_n == 0){
        std::cout << "Please provide a positive number for n_samples. It should be around the number of points you expect to use for clustering. (Not the centroids!)" << std::endl;
        return 1;
    }

    if(input_m == 0){
        std::cout << "Please provide a positive number for n_representatives" << std::endl;
        return 1;
    }

    if(input_dimension == 0){
        std::cout << "Input dimension cannot be zero." << std::endl;
        return 1;
    }

    //std::cout << dist.value << std::endl;
    //std::cout << std::to_string(dist.dist == Distance::Euclidean) << std::to_string(dist.dist == Distance::Cosine)  << std::to_string(dist.dist == Distance::Correlation)  << std::endl;

    allowable_ram *= (1024*1024*1024); // convert to bytes from GB;
    double allowed_doubles = allowable_ram / sizeof(double); // sizeof gives results in B, so we now effectively know how many points we can accomodate for.



    // How many doubles should be stored?
    double number_of_doubles = compute_number_of_doubles(input_n, input_m, input_dimension);
    if(number_of_doubles > allowed_doubles){
        // Find a sufficient M such that the requirements can be satisfied.
        // Two cases : either input_m is bigger than 200 or it is smaller
        unsigned int new_m = 0;
        for(unsigned int i = input_m; i > 0; i--){
            number_of_doubles = compute_number_of_doubles(input_n, i, input_dimension);
            if(number_of_doubles < allowed_doubles) {
                new_m = i;
                break;
            }
        }
        if(new_m == 0){
            std::cout << "The provided RAM requirements cannot be satisfied for any number of representatives with the provided dimensions" << std::endl;
        } else {
            std::string plural= new_m==1 ? "" : "s";
            std::cout << "The provided RAM requirements cannot be satisfied for " + std::to_string(input_m) + " representatives but could be for " + std::to_string(new_m) + " representative" + plural + "." << std::endl;
        }
        return 1;
    }
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PAIR);
    std::cout << "Binding to : " << "ipc:///tmp/feeds/0" + std::to_string(port) << std::endl;
    socket.bind("ipc:///tmp/test");

    ServerMessaging::runServer(socket, input_n, input_m, dist.dist);

    std::cout << "Exiting" << std::endl;

    socket.close();
    context.close();
    return 0;
}
