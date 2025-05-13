NOTES:

- need to find some way to make handle index into groups / queue array as a faux pointer (with index / group name) to avoid lvalue refs.
- HOWERVER, this will not work when the work resolves in the same call stack since by the time handle is returned it's considered "done" ... so not sure what to do here.