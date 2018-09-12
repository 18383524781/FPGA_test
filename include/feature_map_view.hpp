/***********************************************************
���ͷ�ļ��е����ݿ��Բο�caffeԴ����
..\caffe-master\examples\cpp_classification\cpp_classification.cpp�ļ�
***********************************************************/
#define USE_OPENCV 1
#define CPU_ONLY 1

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <caffe/caffe.hpp>

#include <algorithm>
#include <iosfwd>
#include <memory>   //ʹ������ָ��
#include <utility>
#include <vector>



using namespace caffe;  // NOLINT(build/namespaces)
using std::string;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;
/***********************************************************
Classifier��������Ҫ�����ǳ�ʼ����һ��CNN�������������Ѿ�����ѵ���õ�ģ�ͣ�
����ǰ�򴫲��㷨����Ҫ����Ԥ������ݼ����ߵ�������ͼƬ����Ԥ�⡣
***********************************************************/
class Classifier {
public:
	/*************************************************
	������캯�����ڶ���ҪԤ������ݼ�������Ԥ��
	����ͼƬ��һ��ΪLeveDB LMDB���ݿ��ļ�
	***********************************************/
	Classifier(const string& model_file, const string& trained_file);

	/****������캯�����ڶԵ�����һ��OpenCV�����ͼƬ����Ԥ��*******/
	int Classify(const cv::Mat& img);

	/*************************************************
	���ڶ�����ͼƬ����ĳһ���feature map�Ŀ��ӻ�
	***********************************************/
	cv::Mat visualize_featuremap(const cv::Mat& img, string layer_name);
private:
	//void SetMean(const string& mean_file);

	/***********************************************
	�������������һ�Ų���ͼƬ��Ȼ�����Ͱ�����ͼƬ���������������һ��ǰ�򴫲���
	Ȼ���ٰ������������أ�Ҳ���ǽ�һ��ͼƬ��ӳ���һ����������
	*************************************************/
	std::vector<int> Predict(const cv::Mat& img);

	void WrapInputLayer(std::vector<cv::Mat>* input_channels);

	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);



private:
	shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	//cv::Mat mean_;
	std::vector<string> labels_;
};




