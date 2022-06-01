#include <zmq.hpp>
#include "src/network/ServerMessaging.h"
#include <iostream>
#include <boost/program_options.hpp>

namespace po=boost::program_options;

int main(int argc, char **argv) {
    unsigned int input_m;
    unsigned int input_n;
    unsigned int input_dimension;
    unsigned int port;
    double allowable_ram;

    po::options_description desc("Launches a server to which it is possible to send a stream of points, maintaining the coreset representation described  in  Ackermann, Marcel R., et al. \"Streamkm++ a clustering algorithm for data streams.\" Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1."
                                 "\nShortly, the idea is to use a tree to partition space around representative points, which serve as proxy-centroids."
                                 "When one wishes to perform k-means, an approximate solution can be obtained using the representative points, thereby greatly reducing computational costs."
                                 "\nThe server allows to recover at any time either the representatives, or the centroids for a specified number of clusters (performing Kmeans++ in the latter case)."
                                 " Server can also be stopped upon client request, and they communicate via a localhost TCP port."
                                 "\nAllowed options are");
    desc.add_options()
            ("help", "display this help message")
            ("n_samples", po::value<unsigned int>(&input_n)->default_value(250000),
                    "Approximate number of points to pass to the clustering. Does not need to be exact,"
                    "serves to estimate how many buckets to use when performing streamed clustering. Because the relationship"
                    "is logarithmic, the order of magnitude is sufficient. Defaults to 250000 points.")
            ("n_representatives", po::value<unsigned int>(&input_n)->default_value(200*20),
                    "Number of representative points. In other words, how many points will be kept to "
                    "perform clustering operations. A good approximation is around 200k, k the number of centroids you expect."
                    "Defaults to 4000, ie expects to work up to 20 centroids by default. Set this value if you expect more centroids.")
            ("dim", po::value<unsigned int>(&input_dimension)->default_value(90000),
                    "Expected dimensionality of points. This is just an estimation, so as to estimate if RAM requirements can be fulfilled and in no way will constrain the server. Defaults to 90000.")
            ("ram_limit", po::value<double>(&allowable_ram)->default_value(8.0),
                    "Allowable RAM memory, in giga bytes (GB) to be used over the course of the algorithm."
                    " Note that we expect 1KB = 1024B for example. By default, uses 8GB of RAM. If the algorithm, "
                    "because of the number of samples and number of representatives should fail to meet the RAM limit, "
                    "the program will raise an exception, indicating a sufficient number of representatives that would fulfill RAM requirement.")
            ("port", po::value<unsigned int>(&port)->default_value(5555), "The port on which the server is listening for client requests.");
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if(allowable_ram <= 0 ){
        throw std::invalid_argument("Please provide a positive number for the RAM.");
    }

    if(input_n == 0){
        throw std::invalid_argument("Please provide a positive number for n_samples. It should be around the number of points you expect to use for clustering. (Not the centroids!)");
    }

    if(input_m == 0){
        throw std::invalid_argument("Please provide a positive number for n_representatives");
    }

    if(input_dimension == 0){
        throw std::invalid_argument("Input dimension cannot be zero.");
    }

    allowable_ram *= (1024*1024*1024); // convert to bytes;
    double allowed_doubles = allowable_ram / sizeof(double);



    double number_of_doubles = (ceil(log2(1.0 * input_n / input_m) + 2) + 1) * input_m * input_dimension;
    if(number_of_doubles > allowed_doubles){
        // Find a sufficient M such that the requirements can be satisfied.
        // Two cases : either input_m is bigger than 200 or it is smaller
        int new_m = -1;
        for(unsigned int i = input_m; i > 0; i--){
            number_of_doubles = (ceil(log2(1.0 * input_n / i) + 2) + 1) * i * input_dimension;
            if(number_of_doubles < allowed_doubles) {
                new_m = i;
            }
        }
        if(new_m == -1){
            throw std::invalid_argument("The provided RAM requirements cannot be satisfied for any number of representatives with the provided dimensions");
        } else {
            throw std::invalid_argument("The provided RAM requirements cannot be satisfied for " + std::to_string(input_m) + " representatives but could be for " + std::to_string(new_m) + ".");
        }
    }
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:" + std::to_string(port));

    // What if that is not the case ?
    // In that case, let's simply decrease M until it becomes the case.
    /*
     * Means that 3 + log2(N) >= R/S
     * Means that exp(3 + log2(N)) >= exp(R/S)
     * N*exp(3) >= exp(R/S)
     * N >= exp(R/S - 3)
     * Absolutely unreasonable.
     */
    ServerMessaging::runServer(socket, input_n, input_m);

    socket.close();
    context.close();
    return 0;
}
