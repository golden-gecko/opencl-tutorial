#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

void randomizeArray(cl_int* data, size_t vectorSize)
{
    for (size_t i = 0; i < vectorSize; ++i) 
    {
        data[i] = rand() % 10;
    }
}

int main()
{
    try
    {
        // Allocate and initialize host arrays
        size_t vectorSize = 32;
        size_t localWorkSize = 8;

        cl_int* a = new cl_int[vectorSize];
        cl_int* b = new cl_int[vectorSize];
        cl_int* c = new cl_int[vectorSize];

        randomizeArray(a, vectorSize);
        randomizeArray(b, vectorSize);

        // Get platforms.
        cl::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        for (cl::vector<cl::Platform>::iterator i = platforms.begin(); i != platforms.end(); ++i)
        {
            cl::Platform platform = *i;
            std::string name;

            platform.getInfo(CL_PLATFORM_NAME, &name);

            std::cout << "Name:\t\t" << name << std::endl;

            platform.getInfo(CL_PLATFORM_VENDOR, &name);

            std::cout << "Vendor:\t\t" << name << std::endl;

            std::cout << std::endl;
        }

        // Create a context.
        cl_context_properties cps[3] =
        { 
            CL_CONTEXT_PLATFORM, 
            (cl_context_properties)(platforms[1])(), 
            0 
        };

        cl::Context context(CL_DEVICE_TYPE_GPU, cps);
 
        // Get devices.
        cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
 
        for (cl::vector<cl::Device>::iterator i = devices.begin(); i != devices.end(); ++i)
        {
            cl::Device device = *i;
            std::string name;

            device.getInfo(CL_DEVICE_NAME, &name);

            std::cout << "Name:\t\t" << name << std::endl;

            device.getInfo(CL_DEVICE_VENDOR, &name);

            std::cout << "Vendor:\t\t" << name << std::endl;

            device.getInfo(CL_DEVICE_VERSION, &name);

            std::cout << "Version:\t" << name << std::endl;

            std::cout << std::endl;
        }

        // Create a command queue.
        cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
 
        // Read the OpenCL kernel in from source file.
        std::ifstream file(".\\bin\\Add.cl", std::ifstream::in);
        std::string sourceCode(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));

        cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
 
        // Create program.
        cl::Program program = cl::Program(context, source);
 
        // Build program.
        program.build(devices);
 
        // Create kernel.
        cl::Kernel kernel(program, "Add");
 
        // Allocate the OpenCL buffer memory objects for source and result on the device.
        cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY , sizeof(cl_int) * vectorSize);
        cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY , sizeof(cl_int) * vectorSize);
        cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * vectorSize);
 
        // Asynchronous write of data to GPU device.
        queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, sizeof(cl_int) * vectorSize, a);
        queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, sizeof(cl_int) * vectorSize, b);
 
        // Set the Argument values.
        kernel.setArg(0, bufferA);
        kernel.setArg(1, bufferB);
        kernel.setArg(2, bufferC);
        kernel.setArg(3, vectorSize);
 
        // Launch kernel.
        cl::NDRange global(vectorSize);
        cl::NDRange local(8);

        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
 
        // Read back results and check accumulated errors.
        queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, sizeof(cl_int) * vectorSize, c);
 
        // Print results.
        for (size_t i = 0; i < vectorSize; ++i)
        {
            std::cout << a[i] << " + " << b[i] << " = " << c[i] << std::endl;
        }
    }
    catch (cl::Error error)
    {
        std::cout << error.what() << "(" << error.err() << ")" << std::endl;
    }

    return 0;
}
