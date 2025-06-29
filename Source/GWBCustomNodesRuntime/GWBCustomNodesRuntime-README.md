# GWBEditor Module

Contains runtime support tooling for our custom Gameplay Work Balancer K2 nodes.

### Relevant Classes
* **UGWBWildcardValueCache** is a global static function library that provides methods for custom K2 nodes to "capture" values of variables for latent nodes. 
  * A good example of where this is useful is if we have a loop and a latent K2 node that takes the index of the loop and passes it through as an output intended to be used by a latent exec pin. Without the "captured" value this library provides, the outgoing latent exec pin would always get the last loop index.