#include "psf.h"

#include <string>

int main(int argc, char *argv[]) {
    char * filename = argv[1];

    try{
        PSFDataSet data(filename); 
        data.open();
    

        PSFDoubleVector* vout = (PSFDoubleVector *) data.get_signal_vector("i(vvdd)");

        // for (auto prop: data.get_header_properties()){
        //     std::cout << prop.first << ": \t" << *prop.second << std::endl;
        // }
        
        // for (auto i = vout->begin(); i != vout->end(); ++i)
        //     std::cout << *i << ' ';
        
        std::cout << "\nNumber of time points = " << vout->size() << std::endl;
        data.close();
    }catch (IncorrectChunk &exc){
        std::cerr << "Exception caught " << exc.what() << "\n";
    }
}
