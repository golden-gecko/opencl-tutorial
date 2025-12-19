#include <iostream>
#include <fstream>

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

static void randomizeArray(cl_int* data, size_t vectorSize)
{
    for (size_t i = 0; i < vectorSize; ++i) 
    {
        data[i] = rand() % 10;
    }
}

static void printResults(cl_int* a, cl_int* b, cl_int* c, size_t vectorSize)
{
    for (size_t i = 0; i < vectorSize; ++i)
    {
        std::cout << a[i] << " + " << b[i] << " = " << c[i] << std::endl;
    }
}

int main()
{
    std::srand(time(nullptr));

    // Allocate and initialize host arrays.
    size_t vectorSize = 16;
    size_t localWorkSize = 8;

    cl_int* a = new cl_int[vectorSize];
    cl_int* b = new cl_int[vectorSize];
    cl_int* c = new cl_int[vectorSize];

    randomizeArray(a, vectorSize);
    randomizeArray(b, vectorSize);

    cl_int error = CL_SUCCESS;

    // Get platform number.
    cl_uint platformNumber = 0;

    error = clGetPlatformIDs(0, NULL, &platformNumber);

    if (0 == platformNumber)
    {
        std::cout << "No OpenCL platforms found." << std::endl;

        return 0;
    }

    // Get platform identifiers.
    cl_platform_id* platformIds = new cl_platform_id[platformNumber];

    error = clGetPlatformIDs(platformNumber, platformIds, NULL);

    // Get platform info.
    for (cl_uint i = 0; i < platformNumber; ++i)
    {
        char name[1024] = { '\0' };

        std::cout << "Platform:\t" << i << std::endl;

        error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, 1024, &name, NULL);

        std::cout << "Name:\t\t" << name << std::endl;

        error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_VENDOR, 1024, &name, NULL);

        std::cout << "Vendor:\t\t" << name << std::endl;

        std::cout << std::endl;
    }

    // Get device count.
    cl_uint deviceNumber;

    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceNumber);

    if (0 == deviceNumber)
    {
        std::cout << "No OpenCL devices found on platform " << 1 << "." << std::endl;
    }

    // Get device identifiers.
    cl_device_id* deviceIds = new cl_device_id[deviceNumber];

    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, deviceNumber, deviceIds, &deviceNumber);

    // Get device info.
    for (cl_uint i = 0; i < deviceNumber; ++i)
    {
        char name[1024] = { '\0' };

        std::cout << "Device:\t\t" << i << std::endl;

        error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, 1024, &name, NULL);

        std::cout << "Name:\t\t" << name << std::endl;

        error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VENDOR, 1024, &name, NULL);

        std::cout << "Vendor:\t\t" << name << std::endl;

        error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VERSION, 1024, &name, NULL);

        std::cout << "Version:\t" << name << std::endl;
    }

    std::cout << std::endl;

    // Create the OpenCL context.
    cl_context context = clCreateContext(0, deviceNumber, deviceIds, NULL, NULL, NULL);

    if (NULL == context)
    {
        std::cout << "Failed to create OpenCL context." << std::endl;
    }

    // Create a command-queue
    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceIds[0], 0, &error);

    // Allocate the OpenCL buffer memory objects for source and result on the device.
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(cl_int) * vectorSize, NULL, &error);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(cl_int) * vectorSize, NULL, &error);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * vectorSize, NULL, &error);

    // Read the OpenCL kernel in from source file.
    std::ifstream file(".\\bin\\Add.cl", std::ifstream::in);
    std::string str;

    file.seekg(0, std::ios::end);
    size_t programSize = (size_t)file.tellg();

    str.reserve((unsigned int)file.tellg());
    file.seekg(0, std::ios::beg);
    
    str.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    const char* source = str.c_str();

    // Create the program.
    cl_program program = clCreateProgramWithSource(context, 1, &source, &programSize, &error);

    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the kernel.
    cl_kernel kernel = clCreateKernel(program, "add", &error);

    // Set the argument values.
    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bufferA);
    error = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&bufferB);
    error = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC);

    // Asynchronous write of data to GPU device.
    error = clEnqueueWriteBuffer(commandQueue, bufferA, CL_FALSE, 0, sizeof(cl_int) * vectorSize, a, 0, NULL, NULL);
    error = clEnqueueWriteBuffer(commandQueue, bufferB, CL_FALSE, 0, sizeof(cl_int) * vectorSize, b, 0, NULL, NULL);

    // Launch kernel.
    error = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &vectorSize, &localWorkSize, 0, NULL, NULL);

    // Read back results and check accumulated errors.
    error = clEnqueueReadBuffer(commandQueue, bufferC, CL_TRUE, 0, sizeof(cl_int) * vectorSize, c, 0, NULL, NULL);

    printResults(a, b, c, vectorSize);

    // Cleanup and free memory.
    clFlush(commandQueue);
    clFinish(commandQueue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    delete[] a;
    delete[] b;
    delete[] c;

    delete[] platformIds;
    delete[] deviceIds;

    return 0;
}
