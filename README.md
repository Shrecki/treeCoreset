<div id="top"></div>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->



<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

[![Build Status](https://github.com/Shrecki/treeCoreset/actions/workflows/helloAction.yml/badge.svg?branch=main)](https://github.com/Shrecki/treeCoreset/actions/workflows/helloAction.yml)
[![Coverage Badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/Shrecki/fcde53f4e8d03ff100637910fc249bfc/raw/treeCoreset__main.json)](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/Shrecki/fcde53f4e8d03ff100637910fc249bfc/raw/treeCoreset__main.json)


<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Shrecki/treeCoreset">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">treeCoreset</h3>

  <p align="center">
    An implementation of the stream kmeans++ algorithm described in *modern C++ (Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.)*.
    The main program launches a server, to which points can be sent from other processes arbitrarily. Representatives and centroids can be queried from the server via simple commands.
    Communication relies on the ØMQ library for minimum overhead.
    The server estimates overall footprint on RAM for user input, and if not satisfiable exits and suggests modifications to input parameters to satisfy the specified RAM constraints. Note that the constraint does not consider virtual memory and as such is much stricter than what might be in effect available on your machine.
    <br />
    <a href="https://github.com/Shrecki/treeCoreset"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/Shrecki/treeCoreset">View Demo</a>
    ·
    <a href="https://github.com/Shrecki/treeCoreset/issues">Report Bug</a>
    ·
    <a href="https://github.com/Shrecki/treeCoreset/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

[![Product Name Screen Shot][product-screenshot]](https://example.com)

<p align="right">(<a href="#top">back to top</a>)</p>



### Built With


* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [Boost](https://www.boost.org/)
* [ØMQ](https://zeromq.org/)
* [cmake 3.20 or higher](https://cmake.org/)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This is an example of how you may give instructions on setting up your project locally.
To get a local copy up and running follow these simple example steps.

### Prerequisites

Eigen, Boost and ØMQ are required for this project to work. Eigen is already included as a submodule.


### Installation

0. Install prerequisite libraries:
   ```sh
   sudo apt-get install -y build-essential g++ python-dev autotools-dev libicu-dev libbz2-dev lcov libcppunit-dev software-properties-common
   ```
1. Clone the repo
   ```sh
   git clone https://github.com/Shrecki/treeCoreset.git
   ```
2. Install the submodules:
  ```sh
  git submodule init
  git submodule update
  ```
3. If your cmake version is outdated, install a newer version, e.g:
  ```sh
  wget https://github.com/Kitware/CMake/releases/download/v3.22.5/cmake-3.22.5.tar.gz
  tar -zxvf cmake-3.22.5.tar.gz
  cd cmake-3.22.5
  sudo ./bootstrap
  sudo make
  sudo make install
  cmake --version
  ```

4. Install ØMQ, either from source file or through apt:
  ```sh
  sudo apt-get install -y libzmq3-dev
  ```

5. Install Boost:
  ```sh
  sudo apt-get install -y libboost-all-dev
  wget -O boost_1_79_0.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.gz
  tar xzvf boost_1_79_0.tar.gz
  cd boost_1_79_0/
  ./bootstrap.sh --prefix=/usr/
  ./b2
  sudo ./b2 install
  ```
6. Finally, make the project:
   ```sh
   mkdir cmake-build-debug-coverage
   cd cmake-build-debug-coverage
   cmake ../ -DCODE_COVERAGE=ON
   cmake --build . --target unit_test --config Release -- -j
   ```

Note that in CMakeList, march=native is used. If you're on an unsupported platform, you can remove this flag or - better yet - replace it with the flag corresponding to your CPU architecture to benefit from Eigen's maximum performance.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

The server expecting the stream can be started by running the main program:
   ```sh
   ./treeCoreset
   ```

For the complete set of instructions, type
   ```sh
   ./treeCoreset --help
   ```

Once the server is launched, any other process can send points to the server, as if part of a stream, request representatives, centroids or even stop the server.
For convenience, functions in MATLAB are already provided for the client-side. Equivalent functions in Python are planned for future release.


Should you wish to implement a communication in an unsupported language, here are relevant details to consider. For any transmission to happen between client and server, the client must fulfill the following conditions:
1. Be the only client connected to the server (any new connection will be refused by the server while a client is still attached to the socket).
2. Connect to the ipc socket specified by the user, using ØMQ's API. By default, this socket is ipc:///tmp/sock-0. This connection can only be of type ZMQ_PAIR.
3. Send an appropriate request to the server


<p align="right">(<a href="#top">back to top</a>)</p>




<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the GNU License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Fabrice Guibert - [@twitter_handle](https://twitter.com/twitter_handle) - fabrice.guibert@epfl.ch

Project Link: [https://github.com/Shrecki/treeCoreset](https://github.com/Shrecki/treeCoreset)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* []()
* []()
* []()

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/Shrecki/treeCoreset.svg?style=for-the-badge
[contributors-url]: https://github.com/Shrecki/treeCoreset/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Shrecki/treeCoreset.svg?style=for-the-badge
[forks-url]: https://github.com/Shrecki/treeCoreset/network/members
[stars-shield]: https://img.shields.io/github/stars/Shrecki/treeCoreset.svg?style=for-the-badge
[stars-url]: https://github.com/Shrecki/treeCoreset/stargazers
[issues-shield]: https://img.shields.io/github/issues/Shrecki/treeCoreset.svg?style=for-the-badge
[issues-url]: https://github.com/Shrecki/treeCoreset/issues
[license-shield]: https://img.shields.io/github/license/Shrecki/treeCoreset.svg?style=for-the-badge
[license-url]: https://github.com/Shrecki/treeCoreset/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username
[product-screenshot]: images/screenshot.png
