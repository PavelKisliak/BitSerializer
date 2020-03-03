cmake_minimum_required(VERSION 3.10)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(bitserializer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  core
  archives
)

add_library(bitserializer INTERFACE)

target_include_directories(bitserializer 
  INTERFACE
    $<INSTALL_INTERFACE:core>
    $<INSTALL_INTERFACE:archives>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/archives>
)

if(MSVC)
    target_compile_options(bitserializer INTERFACE
            /permissive- #helps to identify and fix conformance issues in your code,
                        #to make it both more correct and more portable.
            /W4     # all reasonable warnings
            /w14242 #'identfier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 #'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            /w14263 #'function': member function does not override any base class virtual member function
            /w14265 #'classname': class has virtual functions, but destructor is not virtual instances of
                    #this class may not be destructed correctly
            /w14287 #'operator': unsigned/negative constant mismatch
            /we4289 #nonstandard extension used: 'variable': loop control variable declared
                    #in the for-loop is used outside the for-loop scope
            /w14296 #'operator': expression is always 'boolean_value'
            /w14311 #'variable': pointer truncation from 'type1' to 'type2'
            /w14545 #expression before comma evaluates to a function which is missing an argument list
            /w14546 #function call before comma missing argument list
            /w14547 #'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 #'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 #expression has no effect; expected expression with side-effect
            /w14619 #pragma warning: there is no warning number 'number'
            /w14640 #Enable warning on thread un-safe static member initialization
            /w14826 #Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14928 #illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    )
else()
    target_compile_options(bitserializer
            INTERFACE
            -Wall
            -Wextra # reasonable and standard
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                               # non-virtual destructor.

            # track down memory errors
            -Wold-style-cast # warn for c-style casts
            -Wuseless-cast   # warn if you perform a cast to the same type
            -Wcast-align     # warn for potential performance problem casts
            -Wunused         # warn on anything being unused
            -Woverloaded-virtual # warn if you overload but not override a virtual
            -Wnull-dereference # warn if a null dereference is detected

            # function
            -Wpedantic # warn if non-standard C++ is used
            -Wconversion # warn on type conversions that may lose data
            -Wsign-conversion # warn on sign conversions
            -Wdouble-promotion # warn if float is implicit promoted to double

            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being where bitwise is used
            -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
            -Wformat=2 # warn on security issues around functions that format output (ie printf)
            -Weffc++ # warning mode.
    )
endif()

add_subdirectory(tests)
add_subdirectory(samples)
