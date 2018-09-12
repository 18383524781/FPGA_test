#ifndef  _WEIGHTS_VIEW_HPP_
#define	 _WEIGHTS_VIEW_HPP_


#include <opencv2/core/core.hpp>            //��������Ϊ������opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "net2cmd.hpp"
  //Ϊ�����������룬��Ҫ����caffe��ͷ�ļ�

inline int map_func(const float value);


inline string dec2hex(u64 i, int width);



/***********************************************************************************************
���ڶ�û���е�feature map�Ŀ��ӻ���
**************************************************************************************************/
cv::Mat visualize_weights(std::string prototxt, std::string caffemodel, int weights_layer_num);



/******************************************************************************
������ȡģ����ÿ��ľ���˵�ά����ϢPW
������ȡÿ��feature map�����������ϢPX
*******************************************************************************/
 void get_params_shape(std::string prototxt, std::string caffemodel, vector<temp> &pw, vector<temp> &px);


 void save_FPGA_w_b(std::string prototxt, std::string caffemodel, std::string txtpath);
 


#endif