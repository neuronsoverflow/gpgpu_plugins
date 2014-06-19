// compile: g++ has_opencl_gpu.cpp -l OpenCL
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <cstdio> // printf


int main(int argc, char** argv)
{
   int err;
   cl_uint platforms;
   cl_platform_id platform = NULL;
   cl_uint num_devices, i;
   cl_device_id* devices = NULL;
   char version[256];
   char device[256];

   // gets the number of platforms and the platform
   err = clGetPlatformIDs(1, &platform, &platforms);
   if (err != CL_SUCCESS)
   {
      return EXIT_FAILURE;
   }

   // get the number of devices
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
   if (err != CL_SUCCESS)
   {
      return EXIT_FAILURE;
   }

   // allocate the device_id
   devices = (cl_device_id*) calloc(sizeof(cl_device_id), num_devices);

   // Obtain the list of devices available on the platform
   //                             CL_DEVICE_TYPE_GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

   // display the supported devices and their versions
   for (i = 0; i < num_devices; i++)
   {
      clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 256, device, NULL);
      fprintf(stdout, "Device %s supports ", device); // device name

      clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 256, version, NULL);
      fprintf(stdout, "%s\n", version); // device version
   }

   free(devices);

   return num_devices ? CL_SUCCESS : EXIT_FAILURE; // 0 = success; 1 = failure
}
