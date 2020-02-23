/*!
 *****************************************************************
 * @file utilities.hpp
 *****************************************************************
 *
 * @note Copyright (c) 2018 Fraunhofer Institute for Manufacturing Engineering and Automation (IPA)
 * @note Project name: ipa_3d_modelling
 * @author Author: Manh Ha Hoang
 *
 * @date Date of creation: 3.2019
 *
 *
*/

#pragma once



#include <ostream>
#include <random>
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

#include <boost/filesystem.hpp>


//! Print text in color in the console. Only works in Linux.
/*!
  Ref: https://misc.flogisoft.com/bash/tip_colors_and_formatting
*/
#define PRINT_RED(...)        \
    {                         \
        printf("\033[1;31m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }

#define PRINT_GREEN(...)      \
    {                         \
        printf("\033[1;32m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }

#define PRINT_YELLOW(...)     \
    {                         \
        printf("\033[1;33m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }

#define PRINT_BLUE(...)       \
    {                         \
        printf("\033[1;34m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }
#define PRINT_MAGENTA(...)    \
    {                         \
        printf("\033[1;35m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }

#define PRINT_CYAN(...)       \
    {                         \
        printf("\033[1;36m"); \
        printf(__VA_ARGS__);  \
        printf("\033[0m");    \
    }
namespace  Utils{

/// The namespace hash_tuple defines a general hash function for std::tuple
/// See this post for details:
///   http://stackoverflow.com/questions/7110301
/// The hash_combine code is from boost
/// Reciprocal of the golden ratio helps spread entropy and handles duplicates.
/// See Mike Seymour in magic-numbers-in-boosthash-combine:
///   http://stackoverflow.com/questions/4948780
/// @ Ref: https://github.com/intel-isl/Open3D/blob/master/src/Open3D/Utility/Helper.h
namespace hash_tuple {

template <typename TT>
struct hash {
    size_t operator()(TT const& tt) const { return std::hash<TT>()(tt); }
};

namespace {

template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
    seed ^= hash_tuple::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
struct HashValueImpl {
    static void apply(size_t& seed, Tuple const& tuple) {
        HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
        hash_combine(seed, std::get<Index>(tuple));
    }
};

template <class Tuple>
struct HashValueImpl<Tuple, 0> {
    static void apply(size_t& seed, Tuple const& tuple) {
        hash_combine(seed, std::get<0>(tuple));
    }
};

}  // unnamed namespace

template <typename... TT>
struct hash<std::tuple<TT...>> {
    size_t operator()(std::tuple<TT...> const& tt) const {
        size_t seed = 0;
        HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
        return seed;
    }
};

}





namespace hash_eigen {

template <typename T>
struct hash : std::unary_function<T, size_t> {
    std::size_t operator()(T const& matrix) const {
        size_t seed = 0;
        for (int i = 0; i < (int)matrix.size(); i++) {
            auto elem = *(matrix.data() + i);
            seed ^= std::hash<typename T::Scalar>()(elem) + 0x9e3779b9 +
                    (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
}

static bool isDirExist(const std::string& filename)
{
    bool result = true;
    std::ifstream _file( filename.c_str(), std::ios::in );
    if(!_file) // it exists
        result = false;
    _file.close();
    return result;
}
static bool createDir(const std::string& path)
{
    //If the directory is existed
  if( isDirExist(path) ){
    return true;
  }
  int system_flag = system( ("mkdir " + path).c_str() );
  if(system_flag == -1){
   PRINT_RED("Utils::createDir: system_flag failed")
           return false;
  }
  return true;
}
static std::string getFileName(const std::string& full_path, bool with_ext)
{
    // fullpath = "/home/wsy/temp.txt";
    size_t _pos = full_path.find_last_of('/');
    std::string filename( full_path.substr(_pos + 1) );
    if (!with_ext)
      filename = filename.substr(0, filename.length() - 4); // remove ext
    return filename;
}

static float GetRandomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(min, max - 0.0001);
    return dist(mt);
}
static std::string GetRandomString(size_t str_len) {
    auto rand_char = []() -> char {
            const char char_set[] = "0123456789abcdefghijklmnopqrstuvwxyz";
            return char_set[((int)std::floor(GetRandomFloat(0.0f, (float)sizeof(char_set) - 1)))];
};
std::string rand_str(str_len, 0);
std::generate_n(rand_str.begin(), str_len, rand_char);
return rand_str;
}
// Get current date/time, format is YYYY-MM-DD.HH-MM-SS
static const std::string currentDateTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;

    oss << std::put_time(&tm,"%Y-%m-%d-%H-%M-%S");
    std::string str = oss.str();
    return str;
}


static void SaveVertexMap(cv::Mat& mat, std::string filename)
{
    std::ofstream f(filename.c_str(), std::ios_base::binary);
    if(!f.is_open())
    {
        std::cerr << "ERROR - SaveVertexMap:" << std::endl;
        std::cerr << "\t ... Could not open " << filename << " \n";
        return ;
    }

    int channels = mat.channels();

    int header[3];
    header[0] = mat.rows;
    header[1] = mat.cols;
    header[2] = channels;
    f.write((char*)header, 3 * sizeof(int));

    float* ptr = 0;
    for(unsigned int row=0; row<(unsigned int)mat.rows; row++)
    {
        ptr = mat.ptr<float>(row);
        f.write((char*)ptr, channels * mat.cols * sizeof(float));
    }
    f.close();
}
static std::string getFileParentDirectory(const std::string &filename) {
    size_t slash_pos = filename.find_last_of("/\\");
    if (slash_pos == std::string::npos) {
        return "";
    } else {
        return filename.substr(0, slash_pos + 1);
    }
}



static void readLabelFromFile(const std::string& label_file, std::vector<int>& face_label)
{
    std::string line;
    std::ifstream file (label_file.c_str());
    if (file.is_open())
    {
        while ( file.good())
        {
            std::getline(file,line);
            std::istringstream iss(line);
            int id;
            iss>>id;
            face_label.push_back(id);
        }
        file.close();
    }
    std::cout<<"facesLable_.size(): "<<face_label.size()<<std::endl;
}



//static void createMveScene(const std::string& output_path, const std::vector<cv::Mat> & image_data_vec,
//                           const std::vector<int>& pose_flags,const std::vector<Eigen::Matrix4d>& transforms,const CameraParameters& cam_params)
//{
//    boost::filesystem::remove_all(output_path+"scene");
//    std::string path=output_path+"scene/views";
//    int index=0;

//    for(int j=0;j<image_data_vec.size();j++)
//    {
//        if (pose_flags[j]!=1) continue;
//        std::stringstream ss;
//        ss<<index++;
//        std::string view_dir=path+"/view"+ss.str()+".mve";
//        boost::filesystem::create_directories(view_dir);
//        cv::imwrite(view_dir+"/original.png",image_data_vec[j]);
//        Eigen::Matrix4d transf=transforms[j].inverse();
//        std::ofstream fs;
//        fs.open (view_dir+"/meta.ini");
//        fs<<"[camera]\n";
//        float focal_length=cam_params.focal_x/cam_params.image_width;
//        fs<<"focal_length = "<<focal_length<<std::endl;
//        fs<<"pixel_aspect = 1"<<std::endl;
//        float px=cam_params.c_x/cam_params.image_width;
//        float py=cam_params.c_y/cam_params.image_height;
//        fs<<"principal_point = "<<px<<" "<<py<<std::endl;
//        fs<<"radial_distortion = 0 0\n";
//        fs<<"rotation = ";
//        for (int r=0;r<3;r++)
//            for (int c=0;c<3;c++)
//                fs<<transf(r,c)<<" ";
//        fs<<std::endl;
//        fs<<"translation = "<<transf(0,3)<<" "<<transf(1,3)<<" "<<transf(2,3)<<std::endl;

//        fs<<"\n[view]\n";
//        fs<<"id = "<<index<<std::endl;
//        fs<<"name = color-"<<index<<std::endl;

//        fs.close();
//    }
//    //run: ./texrecon ../../../../catkin_ws/volkorn/mveScene/scene::"original" ../../../../catkin_ws/volkorn/polygon_mesh.ply original
//}

};



