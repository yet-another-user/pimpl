In the domain of commercial large-scale software development the following design principles become important:

* modularity, separation of concerns,
* implementation hiding,
* minimization of compilation and component dependencies, 
* consistent and recognizable implementation and deployment patterns, 
* multi-platform support.

The Pimpl idiom can help great deal achieving these goals. It is a simple yet robust programming technique to minimize coupling via 

* the separation of public interface and private implementation and then 
* implementation hiding. 

This header-only library provides a deployment pattern and the supporting infrastructure in an attempt

* to minimize the deployment and maintenance hassle and the potential for error,
* to formalize the deployment of the idiom.

**HTML documentation is available [here](http://yet-another-user.github.io/pimpl).**
