__kernel void add(__global int* a, __global int* b, __global int* c)
{
	c[get_global_id(0)] = a[get_global_id(0)] + b[get_global_id(0)];
}
