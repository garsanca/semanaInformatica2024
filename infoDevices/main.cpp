#include <CL/sycl.hpp>
#include <iostream>

using namespace sycl;

int main() {
    std::cout << "List Platforms and Devices" << std::endl;
    std::vector<platform> platforms = platform::get_platforms();
    for (const auto &plat : platforms) {
        // get_info is a template. So we pass the type as an `arguments`.
        std::cout << "Platform: "
            << plat.get_info<info::platform::name>() << " "
            << plat.get_info<info::platform::vendor>() << " "
            << plat.get_info<info::platform::version>() << std::endl;

        std::vector<device> devices = plat.get_devices();
        for (const auto &dev : devices) {
            std::cout << "Device: "
                << dev.get_info<info::device::name>() << std::endl
                << "     is gpu?=" << (dev.is_gpu() ? "Yes" : "No") << std::endl
                << "     Driver version " << dev.get_info<info::device::driver_version>() << std::endl
                << "     Vendor ID " << dev.get_info<info::device::vendor_id>() << std::endl
                << "     Compute Units " << dev.get_info<info::device::max_compute_units>() << std::endl
                << "     Max Work-Group size is " << dev.get_info<info::device::max_work_group_size>() << std::endl
                << "     Max Global mem size is " << dev.get_info<info::device::global_mem_size>() << std::endl
                << "     Max Local mem size is " << dev.get_info<info::device::local_mem_size>() << std::endl
                << std::endl;
        }
    }
    return 0;
}

