#include <string>

#include "psf.h"

int main(int argc, char *argv[]) {
    char *filename = argv[1];

    try {
        PSFDataSet data(filename);
        data.open();

        for (auto prop : data.get_header_properties()) {
            std::cout << prop.first << ": \t" << *prop.second << std::endl;
        }

        for (auto prop : data.get_signal_names()) {
            std::cout << prop << std::endl;
        }

        std::vector<std::string> signal_names;
        if (argc < 3) {
            signal_names.push_back("vin");
        } else {
            for (int i = 2; i < argc; i++) {
                signal_names.push_back(argv[i]);
            }
        }

        // find file name
        std::string f_name = std::string(filename);

        size_t sep_pos = f_name.rfind('/', f_name.length());
        if (sep_pos != std::string::npos) {
            f_name = f_name.substr(sep_pos + 1, f_name.length() - sep_pos);
        }

        for (auto signal_name : signal_names) {
            if (f_name.find("dc") != std::string::npos) {
                std::cout << signal_name << " = " << data.get_signal_scalar(signal_name) << std::endl;
            } else {
                PSFDoubleVector *signal = (PSFDoubleVector *)data.get_signal_vector(signal_name);
                std::cout << signal_name << " Number of time points = " << signal->size() << std::endl;
                for (auto i = signal->begin(); i != signal->end(); ++i)
                    std::cout << *i << ' ';
                std::cout << std::endl;
            }
        }
        data.close();
    } catch (const std::exception &exc) {
        std::cerr << "Exception caught " << exc.what() << "\n";
    }
}
