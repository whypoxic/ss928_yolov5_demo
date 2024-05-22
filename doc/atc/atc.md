### ATC环境的搭建

#### 一.安装conda

#### 二、安装atc相关环境

~~~

conda create -n atc python=3.9.2 
conda env list 
conda config --set auto_activate_base false
conda activate atc

conda deactivate
~~~

#### 三、安装atc相关依赖

~~~
pip3 install protobuf==3.13.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install psutil==5.7.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install numpy --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install scipy --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install decorator==4.4.0 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install sympy==1.5.1 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install cffi==1.12.3 --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install pyyaml --user -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install pathlib2 --user -i https://pypi.tuna.tsinghua.edu.cn/simple

~~~

#### 四、安装atc软件

#### 五、转换模型

YOLOv5 v6.2 export  onnx model method https://github.com/shaoshengsong/yolov5_62_export_ncnn
~~~

source $HOME/Ascend/ascend-toolkit/latest/x86_64-linux/bin/setenv.bash
atc --model=yolov5s_v6.2.onnx --framework=5 --output=yolov5s_v6.2 --soc_version="OPTG" --output_type=FP32 --insert_op_conf=./op.cfg
~~~

##### 5.1 op.cfg yuv420输入配置,输入为640*640 yuv420
~~~
aipp_op {
    aipp_mode : static
    related_input_rank : 0
    max_src_image_size : 1228800
    support_rotation : false
    input_format : YUV420SP_U8
    src_image_size_w : 640
    src_image_size_h: 640
    cpadding_value: 0.0
    crop : false
    load_start_pos_w : 0
    load_start_pos_h : 0
    crop_size_w : 0
    crop_size_h : 0
    resize : false
    resize_output_w : 640
    resize_output_h : 640
    padding : false
    left_padding_size : 0
    right_padding_size : 0
    top_padding_size : 0
    bottom_padding_size : 0
    padding_value : 0
    csc_switch : true
    rbuv_swap_switch : false
    ax_swap_switch : false
    matrix_r0c0 : 256
    matrix_r0c1 : 0
    matrix_r0c2 : 0
    matrix_r1c0 : 0
    matrix_r1c1 : 0
    matrix_r1c2 : 0
    matrix_r2c0 : 0
    matrix_r2c1 : 0
    matrix_r2c2 : 0
    output_bias_0 : 0
    output_bias_1 : 0
    output_bias_2 : 0
    input_bias_0 : 0
    input_bias_1 : 0
    input_bias_2 : 0
    mean_chn_0 : 0
    min_chn_0 : 0.0
    var_reci_chn_0 : 0.00392157
}
~~~