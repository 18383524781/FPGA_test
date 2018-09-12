#include "stdafx.h"
#include "feature_map_view.hpp"
//#include "head.h"


cv::Mat Classifier::visualize_featuremap(const cv::Mat& img, string layer_name)
{
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);

	Preprocess(img, &input_channels);

	net_->Forward();

	//�������ӻ�
	//��ӡ��һ��ͼƬ���������������ĸ������  
	std::cout << "�����е�Blobs����Ϊ��\n";
	vector<shared_ptr<Blob<float> > > blobs = net_->blobs();   //�õ�����������������	
	vector<string> blob_names = net_->blob_names();            //����������������	
	std::cout << blobs.size() << " " << blob_names.size() << std::endl;
	for (int i = 0; i<blobs.size(); i++){
		std::cout << blob_names[i] << " " << blobs[i]->shape_string() << std::endl;
	}
	std::cout << std::endl;

	//������ͼƬ������һ������������ͼ���ӻ�	
	//string blobName = "conv2";   //����ȡ������һ������������ͼ	
	assert(net_->has_blob(layer_name));    //Ϊ��������Ǳ�����ԣ�������ȷʵ������ΪblobName������ͼ
	shared_ptr<Blob<float> >  conv1Blob = net_->blob_by_name(layer_name);   //1*96*55*55    ���Գɹ��󣬰����ַ��ظ� ��������
	std::cout << "����ͼƬ��������Ӧͼ����״��ϢΪ��" << conv1Blob->shape_string() << std::endl;   //��ӡ���������ͼ����״��Ϣ
	//��Ҫ��һ����0~255	

	float maxValue = -10000000, minValue = 10000000;
	const float* tmpValue = conv1Blob->cpu_data();
	for (int i = 0; i<conv1Blob->count(); i++){
		maxValue = std::max(maxValue, tmpValue[i]);
		minValue = std::min(minValue, tmpValue[i]);
	}


	int width = conv1Blob->shape(3);  //��Ӧͼ�ĸ߶�	
	int height = conv1Blob->shape(2);  //��Ӧͼ�Ŀ��	
	int channel = conv1Blob->shape(1);  //ͨ����
	int num = conv1Blob->shape(0);      //����	
	int imgHeight = (int)(1 + sqrt(channel))*height;
	int imgWidth = (int)(1 + sqrt(channel))*width;
	cv::Mat img(imgHeight, imgWidth, CV_8UC1, cv::Scalar(0));   //��ʱ��Ӧ���ǻҶ�ͼ

	int kk = 0;
	for (int x = 0; x<imgHeight; x += height){
		for (int y = 0; y<imgWidth; y += width){
			if (kk >= channel)
				continue;
			cv::Mat roi = img(cv::Rect(y, x, width, height));
			for (int i = 0; i<height; i++){
				for (int j = 0; j<width; j++){
					float value = conv1Blob->data_at(0, kk, i, j);
					roi.at<uchar>(i, j) = (value - minValue) / (maxValue - minValue) * 255;
				}
			}
			kk++;
		}
	}


	return img;


}


Classifier::Classifier(const string& model_file,
	const string& trained_file)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

}

/* Return the top N predictions. */
int Classifier::Classify(const cv::Mat& img) {
	std::vector<int> output = Predict(img);
	std::vector<int>::iterator iter = find(output.begin(), output.end(), 1);
	int prediction = distance(output.begin(), iter);
	return prediction<10 ? prediction : 0;
}
std::vector<int> Classifier::Predict(const cv::Mat& img) {
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_,
		input_geometry_.height, input_geometry_.width);
	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);

	Preprocess(img, &input_channels);

	net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	return std::vector<int>(begin, end);
}

void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i) {
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += width * height;
	}
}

void Classifier::Preprocess(const cv::Mat& img,
	std::vector<cv::Mat>* input_channels) {
	/* Convert the input image to the input image format of the network. */
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
	else
		sample = img;

	cv::Mat sample_resized;
	if (sample.size() != input_geometry_)
		cv::resize(sample, sample_resized, input_geometry_);
	else
		sample_resized = sample;

	cv::Mat sample_float;
	if (num_channels_ == 3)
		sample_resized.convertTo(sample_float, CV_32FC3);
	else
		sample_resized.convertTo(sample_float, CV_32FC1);

	cv::split(sample_float, *input_channels);

	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
		== net_->input_blobs()[0]->cpu_data())
		<< "Input channels are not wrapping the input layer of the network.";
}




