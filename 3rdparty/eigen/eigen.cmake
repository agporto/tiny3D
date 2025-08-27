include(ExternalProject)

# define a proper download directory
set(TINY3D_THIRD_PARTY_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/3rdparty_downloads")
set(EIGEN_PREFIX_DIR "${CMAKE_BINARY_DIR}/3rdparty/eigen")

ExternalProject_Add(
    ext_eigen
    PREFIX ${EIGEN_PREFIX_DIR}
    URL https://gitlab.com/libeigen/eigen/-/archive/da7909592376c893dabbc4b6453a8ffe46b1eb8e/eigen-da7909592376c893dabbc4b6453a8ffe46b1eb8e.tar.gz
    URL_HASH SHA256=37f71e1d7c408e2cc29ef90dcda265e2de0ad5ed6249d7552f6c33897eafd674
    DOWNLOAD_DIR "${TINY3D_THIRD_PARTY_DOWNLOAD_DIR}/eigen"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    ${EP_RELEASE}
)

ExternalProject_Get_Property(ext_eigen SOURCE_DIR)
set(EIGEN_INCLUDE_DIRS ${SOURCE_DIR}/Eigen)
