#include <iostream>
#include <fstream>

#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CLHPP/opencl.hpp>

static void printPlatforms(cl::vector<cl::Platform>& platforms)
{
    for (cl::vector<cl::Platform>::iterator i = platforms.begin(); i != platforms.end(); ++i)
    {
        std::string name;
        std::string vendor;

        i->getInfo(CL_PLATFORM_NAME, &name);
        i->getInfo(CL_PLATFORM_VENDOR, &vendor);

        std::cout << "Name:\t\t" << name << "\n";
        std::cout << "Vendor:\t\t" << vendor << "\n\n";
    }
}

static void printDevices(cl::vector<cl::Device>& devices)
{
    for (cl::vector<cl::Device>::iterator i = devices.begin(); i != devices.end(); ++i)
    {
        std::string name;
        std::string vendor;
        std::string version;

        i->getInfo(CL_DEVICE_NAME, &name);
        i->getInfo(CL_DEVICE_VENDOR, &vendor);
        i->getInfo(CL_DEVICE_VERSION, &version);

        std::cout << "Name:\t\t" << name << "\n";
        std::cout << "Vendor:\t\t" << vendor << "\n";
        std::cout << "Version:\t" << version << "\n\n";
    }
}

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

    // Get platform identifiers.
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    printPlatforms(platforms);

    // Get device identifiers.
    cl::vector<cl::Device> devices;

    platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

    printDevices(devices);

    // Create the OpenCL context.
    cl::Context context({ devices[0]});


    // Read the OpenCL kernel in from source file.
    std::ifstream file(".\\bin\\Add.cl", std::ifstream::in);
    std::string sourceCode(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));

    // Create the kernel.
    cl::Program::Sources sources;
    sources.push_back({ sourceCode.c_str(),sourceCode.length() });

    cl::Program program(context, sources);

    if (program.build({ devices[0]}) != CL_SUCCESS) {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << "\n";
        exit(1);
    }

    // Create buffers on the device.
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(cl_int) * vectorSize);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(cl_int) * vectorSize);
    cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(cl_int) * vectorSize);

    // Create queue to which we will push commands for the device.
    cl::CommandQueue queue(context, devices[0]);

    // Write arrays A and B to the device.
    queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(cl_int) * vectorSize, a);
    queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(cl_int) * vectorSize, b);

    // Run the kernel.
    cl::Kernel kernel_add=cl::Kernel(program, "add");

    kernel_add.setArg(0, buffer_A);
    kernel_add.setArg(1, buffer_B);
    kernel_add.setArg(2, buffer_C);

    queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(vectorSize),cl::NullRange);
    queue.finish();

    // Read result C from the device to array C.
    queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, sizeof(cl_int) * vectorSize, c);

    printResults(a, b, c, vectorSize);

    delete[] a;
    delete[] b;
    delete[] c;

    return 0;
}
