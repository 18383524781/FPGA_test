#define USE_OPENCV 1
#define CPU_ONLY 1

#include "stdafx.h"
#include "weights_view.hpp"
#include <caffe/caffe.hpp>    //Ϊ�����������룬��Ҫ����caffe��ͷ�ļ�
#include "caffe.pb.h"
#include <algorithm>
#include <iosfwd>
#include <memory>                  //ʹ��c++����ָ�룬���������Ŀ¼
#include <utility>
#include <math.h>
#include "head.h"




using namespace caffe;  // NOLINT(build/namespaces)
using namespace cv;
//using namespace std;

/******************************************************************************
��һ�����������ڶ����ݵĹ�һ������
*******************************************************************************/
inline int map_func(const float value)
{

	int result;
	result = int(value * 128);
	return result;
}


/******************************************************************************
������ת��Ϊ16���Ƶ��ַ������ַ�������Ϊ16λ�����ַ���ͷ������ϱ�ʶ����ox��
*******************************************************************************/
inline string dec2hex(u64 i, int width)
{
	std::stringstream ioss; //�����ַ�����
	std::string s_temp; //���ת�����ַ�
	ioss << std::hex << i; //��ʮ������ʽ���
	ioss >> s_temp;
	std::string s(width - s_temp.size(), '0'); //��0
	s = "ox"+s+ s_temp; //�ϲ�
	return s;
}

/***********************************************************************************************
���ڶ�û���е�feature map�Ŀ��ӻ���
**************************************************************************************************/
cv::Mat visualize_weights(string prototxt, string caffemodel, int weights_layer_num)
{

	::google::InitGoogleLogging("0");
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	//��ʼ��һ�����磬����ṹ��caffenet_deploy.prototxt�ļ��ж�ȡ��TEST��ʾ�ǲ��Խ׶�
	Net<float> net(prototxt, TEST);
	net.CopyTrainedLayersFrom(caffemodel);   //��ȡ�Ѿ�ѵ���õ�model�Ĳ���
	vector<boost::shared_ptr<Blob<float> > > params = net.params();    //��ȡ����ĸ�����ѧϰ���Ĳ���(Ȩֵ+ƫ��)

	//��ӡ����modelѧ���ĸ���Ĳ�����ά����Ϣ
	std::cout << "���������ά����ϢΪ��\n";
	for (int i = 0; i<params.size(); ++i)
		std::cout << params[i]->shape_string() << std::endl;


	int width = params[weights_layer_num]->shape(3);     //���,��һ�������Ϊ11 
	int height = params[weights_layer_num]->shape(2);    //�߶ȣ���һ�������Ϊ11
	int channel = params[weights_layer_num]->shape(1);		//ͨ����
	int num = params[weights_layer_num]->shape(0);       //����˵ĸ���


	//���ǽ�num��ͼ������ͬһ�Ŵ�ͼ�Ͻ�����ʾ����ʱ��OpenCV���п��ӻ�������һ����ߴ��ͼƬ��ʹ֮���������еľ����ͼ	
	int imgHeight = (int)(1 + sqrt(num))*height;  //��ͼ�ĳߴ�	
	int imgWidth = (int)(1 + sqrt(num))*width;
	Mat img(imgHeight, imgWidth, CV_8UC3, Scalar(0, 0, 0));

	//�����Ȩֵ����һ�������ɸ���ʵ��������OpenCV���һ��ͼƬ��ÿ�����ص�ֵ��0~255֮��	
	//��Ȩֵ���й�һ����0~255����������ʾ
	float maxValue = -1000, minValue = 10000;
	const float* tmpValue = params[weights_layer_num]->cpu_data();   //��ȡ�ò�Ĳ�����ʵ������һ��һά����	
	for (int i = 0; i<params[weights_layer_num]->count(); i++){        //��������Сֵ		
		maxValue = std::max(maxValue, tmpValue[i]);
		minValue = std::min(minValue, tmpValue[i]);
	}
	//��������ʾ�Ĵ�ߴ�ͼƬ������������ظ�ֵ
	int kk = 0;                         //��ʱ�ڻ���kk�������
	for (int y = 0; y<imgHeight; y += height){
		for (int x = 0; x<imgWidth; x += width){
			if (kk >= num)
				continue;
			Mat roi = img(Rect(x, y, width, height));
			for (int i = 0; i<height; i++){
				for (int j = 0; j<width; j++){
					for (int k = 0; k<channel; k++){
						float value = params[weights_layer_num]->data_at(kk, k, i, j);

						roi.at<Vec3b>(i, j)[k] = (value - minValue) / (maxValue - minValue) * 255;   //��һ����0~255
					}
				}
			}
			++kk;
		}
	}
	resize(img, img, Size(500, 500));   //����ʾ�Ĵ�ͼ������Ϊ500*500�ߴ�	
	imshow("conv1", img);              //��ʾ	
	waitKey(0);
	return img;
}


/******************************************************************************
������ȡģ����ÿ��ľ���˵�ά����ϢPW
������ȡÿ��feature map�����������ϢPX
*******************************************************************************/
void get_params_shape(std::string prototxt, std::string caffemodel, vector<temp> &pw, vector<temp> &px)
{
	//��ʼ��һ�����磬����ṹ��caffenet_deploy.prototxt�ļ��ж�ȡ��TEST��ʾ�ǲ��Խ׶�
	Net<float> net(prototxt, TEST);
	net.CopyTrainedLayersFrom(caffemodel);   //��ȡ�Ѿ�ѵ���õ�model�Ĳ���


	vector<boost::shared_ptr<Blob<float> > > params = net.params();    //��ȡ����ĸ�����ѧϰ���Ĳ���(Ȩֵ+ƫ��)
	string para_names = "conv";

	vector<boost::shared_ptr<Blob<float>>> blobs = net.blobs();//�õ�����������������
	vector<string> blob_names = net.blob_names();

	cout << "����˲���Ϊ��" << params.size()/2 << endl;
	for (int i = 0; i < params.size(); ++i)
	{
		cout << "ÿ��ά�ȣ�" << params[i]->num_axes()<< endl;
		temp temp_weights_shape;
		int index = params[i]->num_axes();
		if (i % 2 == 0)
		{
			if (index==4)
			{
				temp_weights_shape.channce = params[i]->shape(0);	                     //ͼƬ����
				temp_weights_shape.num = params[i]->shape(1);                            //ͨ����
				temp_weights_shape.x_size = params[i]->shape(2);                         //���
				temp_weights_shape.y_size = params[i]->shape(3);                         //�߶�
			}
			else if (index == 2)
			{
				temp_weights_shape.channce = params[i]->shape(0);	                     //ͼƬ����
				temp_weights_shape.num = params[i]->shape(1);                            //ͨ����
				temp_weights_shape.x_size = 1;                                           //���
				temp_weights_shape.y_size = 1;                                           //�߶�
			}
			else cout << "���ִ���1" << endl;

			temp_weights_shape.conv = para_names;
			pw.push_back(temp_weights_shape);
		}
	}
	//��ȡfeature map��ά����Ϣ��
	cout << "feature map����Ϊ��" << blobs.size() << endl;
	for (int i = 0; i < blobs.size(); ++i)
	{
		temp temp_feature_shape; 
		int index = blobs[i]->num_axes(); //��ȡÿ���shape��С��ÿ���shape��С��������4ά����2ά
		if (index==4)//����ά�������ȡ����
		{
			temp_feature_shape.num = blobs[i]->shape(0);
			temp_feature_shape.channce = blobs[i]->shape(1);
			temp_feature_shape.x_size = blobs[i]->shape(2);
			temp_feature_shape.y_size = blobs[i]->shape(3);
		}
		else if (index == 2)//�ڶ�ά���������ȡ����
		{
			temp_feature_shape.num = blobs[i]->shape(0);
			temp_feature_shape.channce = blobs[i]->shape(1);
			temp_feature_shape.x_size = 1;
			temp_feature_shape.y_size = 1;
		}
		else cout << "���ִ���2" << endl;//��ά�Ȳ����ϵ�����±���
		temp_feature_shape.conv = blob_names[i];
		px.push_back(temp_feature_shape);
	}
}



/******************************************************************************
����FPGA����ʦҪ��Ĳ�����ʽ��Ƶģ�������ȡ����һ����ת���㷨
*******************************************************************************/


void save_FPGA_w_b(std::string prototxt, std::string caffemodel, std::string txtpath)
{
	::google::InitGoogleLogging("0");
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif
	
	//��ʼ��һ�����磬����ṹ��caffenet_deploy.prototxt�ļ��ж�ȡ��TEST��ʾ�ǲ��Խ׶�
	Net<float> net(prototxt, TEST);
	net.CopyTrainedLayersFrom(caffemodel);   //��ȡ�Ѿ�ѵ���õ�model�Ĳ���
	vector<boost::shared_ptr<Blob<float> > > params = net.params();    //��ȡ����ĸ�����ѧϰ���Ĳ���(Ȩֵ+ƫ��)

	//��ӡ����modelѧ���ĸ���Ĳ�����ά����Ϣ
	std::cout << "���������ά����ϢΪ��\n";
	cout << "wangluode cehngshu " << params.size() << endl;
	for (int i = 0; i<params.size(); ++i)
		std::cout << params[i]->shape_string() << std::endl;

	int size = params.size() / 2;
	
	bool zero_fill;
	int cyc_one,cyc_two,cyc_three,cyc_four,cyc_b;
	int one_index, two_index, three_index, four_index,b_index;
	int value = 0,value_b=0;
	ofstream fp_w_b;
	fp_w_b.open("w_b.txt", ios::out);
	/********************************************************
	1��ÿ�����֡��Ӧ��8������֡�ľ������������һ��ռ��1����ַ��
	2��Ȼ�����֡0~15��˳�򣬰ڷ�(0,0)��Ĳ�������ռ��16����ַ��
	3����1),2)�Ķ��壬�����3x3��������ΰڷ�(0,0),(1,0),(2,0),(0,1),(1,1),(2,1),(0,2),(1,2),(2,2)��ľ����������ռ��144����ַ�������1x1�ľ����ֻ�ڷ�(0,0)��Ĳ�����
	4����1)~3)�Ķ��壬�ڷ�ʣ������֡��8 ~ C_MAX������0��15�����֡�ľ��������
	5����1)~4)�Ķ��壬�ڷ�16 ~ N_MAX�ľ��������
	6���ڰ�ÿһ���Ȩֵ������ȡ�������ȡƫ�ò���
	7����1)~6)�Ķ��壬�ڷ�����ÿ��ľ������
	********************************************************/

	for (int layer_index = 0; layer_index < size; ++layer_index)
	{
		int conv_index = layer_index * 2;

		int w_shanpe = params[conv_index]->num_axes();
		cout << "��һ���shape" << w_shanpe << endl;

		if (w_shanpe != 4) break;
		if (params[conv_index]->shape(1) < 8)
		{
			zero_fill = true;
			cyc_one = 1;
		}
		else
		{
			cyc_one = params[conv_index]->shape(1) / 8;
			zero_fill = false;
		}
		cyc_two = params[conv_index]->shape(0) / 16;
		cyc_b = params[conv_index]->shape(0) / 8;
		cyc_three = params[conv_index]->shape(2);
		cyc_four = params[conv_index]->shape(3);
		for (int k1 = 0; k1 < cyc_one; k1++)
		{
			for (int k2 = 0; k2 < cyc_two; k2++)
			{
				for (int k3 = 0; k3 < cyc_three; k3++)
				{
					for (int k4 = 0; k4 < cyc_four; k4++)
					{
						for (int k5 = 0; k5 < 16; k5++)
						{
							u64 kay_value = 0;
							if (zero_fill == false)
							{
								//��ȡȨ�ز�����������һ������ת��Ϊ16���Ƴ�Ϊ16 λ���ַ���
								//��ȡfeature map����ͨ��Ϊ8 �ı�����Ȩ��
								for (int k6 = 0; k6 < 8; k6++)
								{
									one_index = k2 * 16 + k5;
									two_index = k1 * 8 + k6;
									three_index = k3;
									four_index = k4;
									value = map_func(params[conv_index]->data_at(one_index, two_index, three_index, four_index));
									if (value < 0)value = value + 256;
									u64 temp = value*pow(256, k6);
									kay_value = kay_value + temp;
								}
							}
							else
							{							
								//��ȡfeature map����ͨ����Ϊ8�ı�����Ȩ�أ���������8ͨ��ʱ����ͨ������
								for (int k6 = 0; k6 < 3; k6++)
								{
									one_index = k2 * 16 + k5;
									two_index = k1 * 8 + k6;
									three_index = k3;
									four_index = k4;
									value = map_func(params[conv_index]->data_at(one_index, two_index, three_index, four_index));
									if (value < 0)value = value + 256;
									u64 temp = value*pow(256, k6);
									kay_value = kay_value + temp;
								}
							}
							string temp_string = dec2hex(kay_value, 16);
							fp_w_b << "*((int *)(cfg_addr + " << k5 << "+ * 8)) = " << temp_string << ";" << endl;
						}
						fp_w_b << "cfg_addr += 16*8;" << endl;
					}
				}
			}
		}
		//��ȡƫ�ò���
		for (int kb = 0; kb < cyc_b; kb++)
		{
			u64 kay_value = 0;
			for (int kb1 = 0; kb1 < 8; kb1++)
			{
				
				b_index = kb * 8 + kb1;
				value_b = map_func(params[conv_index + 1]->data_at(b_index,0,0,0));//����һ������
				if (value_b < 0)value_b = value_b + 256;
				u64 temp = value_b*pow(256, kb1);
				kay_value = kay_value + temp;
			}
			string temp_string = dec2hex(kay_value, 16);//ת��Ϊ16���Ƶĳ�Ϊ10λ�ַ���
			
			fp_w_b << "*((int *)(cfg_addr + " << kb << "+ * 8)) = " << temp_string << ";" << endl;
		}
		//fp_w_b << "������ƫ�ò���" << endl;
		fp_w_b << "cfg_addr += " << cyc_b << "*8;" << endl;
	}
	fp_w_b.close();
}
