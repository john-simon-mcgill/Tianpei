project(BlackScholes)

# set source files
set(PROJECT_SRC BlackScholes.cu BlackScholes_gold.cpp)
set(PROJECT_HEADER BlackScholes_kernel.cuh)

# build options
set(GENCODE -gencode=arch=compute_30,code=sm_30 -gencode=arch=compute_35,code=compute_35)
set(GENCODE ${GENCODE} -gencode=arch=compute_20,code=sm_20)
set(GENCODE ${GENCODE} -gencode=arch=compute_10,code=sm_10)

# debug options
set(CUDA_NVCC_FLAGS_DEBUG "-G;-g")
set(CUDA_NVCC_FLAGS_RELEASE "-O2")

if (MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

cuda_add_executable(BlackScholes ${PROJECT_SRC} ${PROJECT_HEADER} OPTIONS ${GENCODE})
install(TARGETS BlackScholes 
        RUNTIME DESTINATION ${INSTALL_PATH})
