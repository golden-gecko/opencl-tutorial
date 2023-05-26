#include <iostream>

#include <CL/cl.h>

int main()
{
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

		// Get device count.
		cl_uint deviceNumber;

		error = clGetDeviceIDs(platformIds[i], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceNumber);

		if (0 == deviceNumber)
		{
			std::cout << "No OpenCL devices found on platform " << i << "." << std::endl;
		}

		// Get device identifiers.
		cl_device_id* deviceIds = new cl_device_id[deviceNumber];

		error = clGetDeviceIDs(platformIds[i], CL_DEVICE_TYPE_GPU, deviceNumber, deviceIds, &deviceNumber);

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

		// Free memory.
		delete[] deviceIds;
	}

	// Free memory.
	delete[] platformIds;

	// Press Enter, to quit application.
	std::cin.ignore();

	return 0;
}
