#ifndef TF_MICRO_CC_
#define TF_MICRO_CC_
//#define TF_LITE_STATIC_MEMORY
#include "tfmicro.h"
#include <tensorflow/lite/micro/kernels/micro_ops.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
//#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"



#include "model_count_insects_final_2_quant.h"
#include <math.h>

static const char *TAG = "TFMICRO";

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  //tflite::MicroInterpreter* interpreter = nullptr;
 // int inference_count = 0;
  
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  
  //constexpr int kTensorArenaSize = saved_models_model_data_tflite_len + (240*240 *1024)+(60*1024);
  constexpr int kTensorArenaSize = 1200*1024;
  
  uint8_t *tensor_arena=(uint8_t *)malloc(kTensorArenaSize*sizeof(uint8_t));
}  // namespace

int getPredict(const uint8_t *image,const size_t width,const size_t height,const size_t channel)
{
  tflite::InitializeTarget(); 
    ESP_LOGI(TAG, "InitializeTarget");
    
    // Set up logging. Google style is to avoid globals or statics because of
    // lifetime uncertainty, but since this has a trivial destructor it's okay.
    // NOLINTNEXTLINE(runtime-global-variables)		
    static tflite::MicroErrorReporter micro_error_reporter;
	  error_reporter = &micro_error_reporter;
    if(error_reporter!= nullptr){
      ESP_LOGI(TAG, "MicroErrorReporter");
    }else{
      ESP_LOGE(TAG, "MicroErrorReporter error_reporter Null");
      return false;
    }
      
    
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    ESP_LOGI(TAG, "model_data_tflite_len %d",sizeof(saved_models_model_data_tflite));

    model = tflite::GetModel(saved_models_model_data_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) 
    {
		  TF_LITE_REPORT_ERROR(error_reporter,
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     model->version(), TFLITE_SCHEMA_VERSION);
      
		  return false;
    }
     
    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver micro_op_resolver;
    ESP_LOGI(TAG, " AllOpsResolver");


  //static tflite::MicroMutableOpResolver<4> micro_op_resolver;
  //micro_op_resolver.AddMaxPool2D();
  //micro_op_resolver.AddConv2D();
  //micro_op_resolver.AddRelu();
  //micro_op_resolver.AddFullyConnected();
  //  ESP_LOGI(TAG, " micro_op_resolver End");
  


    // Build an interpreter to run the model with.
    //static tflite::MicroInterpreter interpreter(
    tflite::MicroInterpreter interpreter(
	  model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    
    ESP_LOGI(TAG, " Build an interpreter End");

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter.AllocateTensors();
    if (allocate_status != kTfLiteOk) 
    {
	    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
      ESP_LOGE(TAG, " AllocateTensors Error");
	    return false;
    }
    ESP_LOGI(TAG, " AllocateTensors End");

    input = interpreter.input(0);

    
    ESP_LOGI("TENSOR SETUP", "input size = %d", input->dims->size);
    ESP_LOGI("TENSOR SETUP", "input size in bytes = %d", input->bytes);
    ESP_LOGI("TENSOR SETUP", "Is input float32? = %s", (input->type == kTfLiteFloat32) ? "true" : "false");
    ESP_LOGI("TENSOR SETUP", "Is input int8? = %s", (input->type == kTfLiteInt8) ? "true" : "false");
    ESP_LOGI("TENSOR SETUP", "Is input uint8? = %s", (input->type == kTfLiteUInt8) ? "true" : "false");
    ESP_LOGI("TENSOR SETUP", "Input data dimentions = %d",input->dims->data[1]);

    output = interpreter.output(0);

    ESP_LOGI("TENSOR SETUP", "output size = %d", output->dims->size);
    ESP_LOGI("TENSOR SETUP", "output size in bytes = %d", output->bytes);
    ESP_LOGI("TENSOR SETUP", "Is output float32? = %s", (output->type == kTfLiteFloat32) ? "true" : "false");
    ESP_LOGI("TENSOR SETUP", "Is output int8? = %s", (output->type == kTfLiteInt8) ? "true" : "false");
    ESP_LOGI("TENSOR SETUP", "Is output uint8? = %s", (output->type == kTfLiteUInt8) ? "true" : "false");

  
    float predict_number = -1;
    if (image==NULL)
    {
      TF_LITE_REPORT_ERROR(error_reporter, "NULL Image");
      predict_number=-1;
    }
    else
    {
      if(input==nullptr){
        ESP_LOGE(TAG,"No Input ");
        return -1;
      }

      float input_scale=input->params.scale;
      int32_t input_zero_point=input->params.zero_point;
      ESP_LOGI(TAG," Input scale = %f zero_point=%d",input_scale,input_zero_point);
      ESP_LOGI(TAG, "iput type=%d  ",input->type);
      for (int i=0; i < width*width*channel ;i++)
      {
        float a=(float)image[i]/(float)255;
        float b=a/(float)input_scale;
        float c=b+input_zero_point;
        input->data.int8[i]=(int8_t)c;
        //if(i<10 ) {
        //  ESP_LOGI(TAG, "a =%f b=%f  c=%f ",a,b,c);
        //  ESP_LOGI(TAG, "input =%d  image[i]=%d ",input->data.int8[i],image[i]);
       // }
      } 
    
      if (kTfLiteOk != interpreter.Invoke()) 
		  {
			  TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
        ESP_LOGI(TAG, "output type=%d  ",output->type);
        predict_number=-1;
		  }
      else{
        float output_scale=output->params.scale;
        int32_t output_zero_point=output->params.zero_point;
        ESP_LOGI(TAG,"Output scale = %f zero_point=%d",output_scale,output_zero_point);
        ESP_LOGI(TAG, "poutput type=%d  ",output->type);
        ESP_LOGI(TAG, "pre dequantized predict_number=%d  ",output->data.int8[0]);
        predict_number=(output->data.int8[0]-output_zero_point)*output_scale;
        ESP_LOGI(TAG, "dequantized predict_number=%f  ",predict_number);
        predict_number=round(predict_number);
        ESP_LOGI(TAG, "Rounded predict_number=%f  ",predict_number);
      }
  }
  return (int)predict_number;

}

#endif //TF_MICRO_CC

