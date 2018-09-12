#ifndef  _NET2CMD_H_
#define	 _NET2CMD_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "base_type.hpp"

#define ifrm_xmax	127
#define	ifrm_ymax	63

#define conv_ymax	32
#define conv_xmax	64

using namespace std;



enum convsize {
	conv1 = 0,
	conv3
};

typedef struct  {
	//-- first conv stage control
	u16		ifrm_width;	//count from 1ͼƬ���
	u16		ifrm_height;//ͼƬ����
	u8		conv_size;//����˴�С���3*3
	bool	conv_pad;//�Ƿ��������
	u8		conv_std;	//�������
	u16		ifrm_num;//����һ�����֡��feature map����Ҫ���ٸ�����֡����1��������Ҫ��8����������
	u32		ifrm_bsptr;//�����������֡����ַ��DDR��ַָ�롣(*iframe_base_ptr)ָ���DDR�ռ����δ�Ÿ�������֡��


	bool	relu_en;//��ΪΪTRUE��һ�������Ϊ��0~127������ΪFlash��һ�������Ϊ��-��
	bool	res_en;//��ΪTRUE feature map���ۼӹ��ܣ��ۼӺ�����Relu����ΪFlashfeature map���ۼӹ���
	u32		convp_bsptr;//��ž��������˲�ϵ���ĵ�ַָ�롣W
	u32		convk_bsptr;//��ž�������K������ÿ�����֡��bias���ĵ�ַָ�롣b
	u32		res_bsptr;//���Feature map�ۼӲ�����ݵĵ�ַָ�롣
	bool	pool_en;//��һ��2*2���ػ���׼λ

	//-- second depth wise conv  / pool stage control
	bool	dw_en;//Depth wise ���ʹ�ܡ�
	bool	dw_pad;//True ����䣬Flash����Ҫ���
	u8		dw_std;	//����
	u32		dwp_bsptr;//���Depth wise���������˲�ϵ���ĵ�ַָ�롣����K������ÿ�����֡��bias����
	bool	dw_relu_en;//1'b1:ʹ��Relu���ڶ��������Ϊ[0~127]��������1'b0:��ʹ�ܣ��ڶ��������Ϊ[-128~127]��������


	//-- frame output ctrl
	u16		ofrm_width;//���֡�Ŀ�
	u16		ofrm_height;//���֡�ĳ�
	u16		ofrm_num;//���֡��ͨ����
	u32		ofrm_bsptr;//���֡��ַָ��
	bool	conv_end;	//0������ȥ��conv common ctrl info���У����о�����㡣1�������������������жϡ�

}cal_ctrl;

typedef struct  {
	//-- first conv stage control
	u8		ifrm_xlen;	//count from 1
	u8		ifrm_ylen;
	u16		ifrm_xoff;	//line offset, pixel based
	u8		conv_size;
	bool	conv_tp;
	bool	conv_bp;
	bool	conv_lp;
	bool	conv_rp;
	u8		conv_std;	//stride: 0, stride_1; 1: stride_2
	u16		ifrm_num;
	u32		ifrm_bsptr;
	u32		ifrm_ioff;	//iframe initial pixel offset related to top-left [0,0] pixel, used for tile
	u32		ifrm_psize;	//iframe pixel size of a frame(not tiled), org_ifrm_xlen*org_ifrm_ylen
	bool	pool_en;
	bool	relu_en;
	bool	res_en;
	u32		convp_bsptr;
	u32		convk_bsptr;
	u32		res_bsptr;
	u16		res_xoff;
	u32		res_ioff;
	u32		res_psize;

	//-- second depth wise conv  / pool stage control
	bool	dw_en;
	u8		dw_ifrm_xlen;
	u8		dw_ifrm_ylen;
	bool	dw_tp;
	bool	dw_bp;
	bool	dw_lp;
	bool	dw_rp;
	u8		dw_std;	//stride: 0, stride_1; 1: stride_2
	u32		dwp_bsptr;
	bool	dw_relu_en;

	//-- frame output ctrl
	u8		ofrm_xlen;
	u8		ofrm_ylen;
	u16		ofrm_xoff;	//line offset, pixel based
	u16		ofrm_num;
	u32		ofrm_bsptr;
	u32		ofrm_ioff;	//oframe initial pixel offset related to top-left [0,0] pixel, used for tile
	u32		ofrm_psize;	//oframe pixel size of a frame(not tiled), org_ofrm_xlen*org_ofrm_ylen
	bool	firstile_layer;
	bool	lastile_layer;
	bool	conv_end;
}layer_ctrl;

typedef struct
{
	string         conv;
	u16            num;
	u16            channce;
	u16            x_size;
	u16            y_size;
}temp;



#endif

