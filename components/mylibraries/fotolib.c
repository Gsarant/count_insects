
#include "fotolib.h"

void clear_image(uint8_t *black_white_img,const int imagerow,const int imagecol,const int half_pad_size){
    int black_pixel=0;
    for(int row=0 ; row<imagerow; row++){
        for(int col=0 ; col<imagecol; col++){
            int i=(row*imagecol)+col;
            if(black_white_img[i]==0){
                int local_changes=0;
                for(int rr=(-half_pad_size) ; rr<=half_pad_size;rr++)
                {
                    for(int cc=(-half_pad_size) ; cc<=half_pad_size;cc++)
                    {
                        if(((row+rr)>=0) && ((row+rr)<imagerow)){
                            if(((col+cc)>=0) && ((col+cc)<imagecol)){
                                int new_index=i+(rr*imagecol)+cc;
                                if((i!=new_index) && (black_white_img[new_index]==0)){
                                    local_changes++;
                                }
                            }
                            
                        }
                    }
                }
                if(local_changes > (int)((half_pad_size*half_pad_size)/2)){
                //if(local_changes > 0){
                //if(local_changes == ((check_area_size*check_area_size)-1)){
                    black_white_img[i]=(uint8_t) 0;
                    black_pixel++;

                }else{
                    black_white_img[i]=(uint8_t) 255;
                }
            } 
        }
    }
    ESP_LOGD(TAGFOTOLIB,"**************** clear_image *****************\n");
    ESP_LOGD(TAGFOTOLIB," Black pixel=%d ",black_pixel);
    ESP_LOGD(TAGFOTOLIB,"**************** *********** *****************\n");

}

void create_last_object(const uint8_t *previous_img,const uint8_t *current_img, uint8_t *object_img,const int imagerow,const int imagecol, const int updown, const int threshold,const int half_pad_size){
    int black_pixel=0;
    int white_pixel=0;
    int max_pixel_updown=0;
    int min_pixel_updown=256;
    int max_pixel=0;
    int min_pixel=256;
    
    for(int row=0 ; row<imagerow; row++){
        for(int col=0 ; col<imagecol; col++){
            int i=(row*imagecol)+col;
            if(max_pixel_updown< abs(previous_img[i]-current_img[i])){
                max_pixel_updown=abs(previous_img[i]-current_img[i]);
            }
            else if (min_pixel_updown > abs(previous_img[i]-current_img[i])){
               min_pixel_updown=abs(previous_img[i]-current_img[i]);
            }

          
            if((abs(previous_img[i]-current_img[i])> updown)){
                int local_changes=0;
                for(int rr=(-half_pad_size) ; rr<=half_pad_size;rr++)
                {
                    for(int cc=(-half_pad_size) ; cc<=half_pad_size;cc++)
                    {
                        if(((row+rr)>=0) && ((row+rr)<imagerow)){
                            if(((col+cc)>=0) && ((col+cc)<imagecol)){
                                int new_index=i+(rr*imagecol)+cc;
                                //ESP_LOGI(TAGFOTOLIB,"col=%d row=%d i=%d rr=%d cc=%d new_index=%d",col,row,i,rr,cc,new_index);
                                if((i!=new_index) && (abs(previous_img[new_index]-current_img[new_index])> updown)){
                                        local_changes++;
                                }
                            }
                            
                        }
                    }
                }
                if(local_changes>(int)(pow((2*half_pad_size),2)/2)){
                    object_img[i]=current_img[i];
                    
                    if(max_pixel< current_img[i]){
                       max_pixel=current_img[i];
                    }
                    else if (min_pixel > current_img[i]){
                        min_pixel=current_img[i];
                    }
                    
                    black_pixel++;
                }else{
                    object_img[i]=(uint8_t) 255;
                    white_pixel++;
                }
            }else{
                object_img[i]=(uint8_t) 255;
                white_pixel++;
            }

        }
    }
    ESP_LOGD(TAGFOTOLIB,"**************** create_black_white_last_object_new *****************\n");
    ESP_LOGD(TAGFOTOLIB," Change pixel=%d White Pixel=%d UpDown=%d Half Pad Size=%d",black_pixel,white_pixel,updown,half_pad_size);
    ESP_LOGD(TAGFOTOLIB," imagerow=%d imagecol=%d ",imagerow,imagecol);
    ESP_LOGD(TAGFOTOLIB," max_pixel_updown=%d  min_pixel_updown=%d",max_pixel_updown,min_pixel_updown);
    ESP_LOGD(TAGFOTOLIB," max_pixel=%d  min_pixel=%d",max_pixel,min_pixel);
    ESP_LOGD(TAGFOTOLIB,"**********************************\n");
};


void create_black_white_image(const uint8_t *current_img, uint8_t *black_white_img,const int imagerow,const int imagecol, const int threshold){
    int black_pixel=0;
    int white_pixel=0;
    int max_pixel=0;
    int min_pixel=256;
    
    for(int row=0 ; row<imagerow; row++){
        for(int col=0 ; col<imagecol; col++){
            int i=(row*imagecol)+col;
            if(max_pixel< current_img[i]){
                max_pixel=current_img[i];
            }else if (min_pixel > current_img[i]){
                min_pixel=current_img[i];
            }
           
            if((current_img[i]> threshold)){
                black_white_img[i]=(uint8_t) 255;
                white_pixel++;
            }else{
                black_white_img[i]=(uint8_t) 255;
                white_pixel++;
            }

        }
    }
    ESP_LOGD(TAGFOTOLIB,"**************** create_black_white_last_object_new *****************\n");
    ESP_LOGD(TAGFOTOLIB," Black pixel=%d White Pixel=%d threshold =%d",black_pixel,white_pixel,threshold);
    ESP_LOGD(TAGFOTOLIB," imagerow=%d imagecol=%d ",imagerow,imagecol);
    ESP_LOGD(TAGFOTOLIB," max_pixel=%d  min_pixel=%d",max_pixel,min_pixel);
    ESP_LOGD(TAGFOTOLIB,"**********************************\n");   
};


void find_changes( const uint8_t *obj_img, Rec_change_img_t *rec_change_img ,const int imagerow,const int imagecol, const int threshold){
    
    rec_change_img->top=241;
    rec_change_img->bottom=-1;
    rec_change_img->left=241;                
    rec_change_img->right=-1;
    rec_change_img->change=0;
    for(int row=0 ; row<imagerow; row++){
        for(int col=0 ; col<imagecol; col++){
            int i=(row*imagecol)+col;
            if(obj_img[i] <= threshold){

                rec_change_img->change++;
               // div_t x_y=div(i,src_image_w);
               // int x=x_y.rem;
                //int y=x_y.quot;
                int x=col;
                int y=row;
                if(y<rec_change_img->top){rec_change_img->top=y;}
                else if(y>rec_change_img->bottom){rec_change_img->bottom=y;}
                
                if(x<rec_change_img->left){rec_change_img->left=x;}
                else if(x>rec_change_img->right){rec_change_img->right=x;}
            }
        }
    }
    ESP_LOGD(TAGFOTOLIB,"\n**************** find_changes *****************");
    ESP_LOGD(TAGFOTOLIB,"top=%d left=%d botto,m=%d right=%d   ",rec_change_img->top,rec_change_img->left,rec_change_img->bottom,rec_change_img->right);
    ESP_LOGD(TAGFOTOLIB,"change=%d  threshold=%d ",rec_change_img->change,threshold);
    ESP_LOGD(TAGFOTOLIB,"**********************************\n\n");   

};

float get_percentage_change(const uint8_t * previous_img,const uint8_t * current_img,const int imagerow,const int imagecol, int updown){
    int difference=0;
      for(int row=0 ; row<imagerow; row++){
        for(int col=0 ; col<imagecol; col++){
  
            if(abs(previous_img[(row*imagecol)+col]-current_img[(row*imagecol)+col])> updown){
                difference++;
            }
            //ESP_LOGI(TAGFOTOLIB, "imagesize=%d    x=%d    difference=%d",imagesize,x,difference);
            //ESP_LOGI(TAGFOTOLIB, "previous_img=%d    x=%d    current_img=%d",previous_img[x],x,current_img[x]);
            //esp_task_wdt_reset();
        }
      }
    float a=(float) difference/imagerow*imagecol;
    //ESP_LOGI(TAGFOTOLIB,"************************percentage_change %f",a);
    return a;
};

int save_file_as_jpg(const char * filename, camera_fb_t * fb){
    ESP_LOGI(TAGFOTOLIB,"Try to save_file_as_jpg");
    uint8_t * _jpg_buf=NULL;
    size_t _jpg_buf_len=0;
    if(fb->format != PIXFORMAT_JPEG){
        bool jpeg_converted = frame2jpg(fb, 50, &_jpg_buf, &_jpg_buf_len);
        if(!jpeg_converted){
            ESP_LOGE(TAGFOTOLIB,"JPEG compression failed");
            return 1;
        }
        ESP_LOGI(TAGFOTOLIB,"JPEG compression success");
    }else{
        _jpg_buf=fb->buf;
        _jpg_buf_len=fb->len;
         ESP_LOGI(TAGFOTOLIB,"is allready JPEG");
    }
    return save_file(filename,_jpg_buf,_jpg_buf_len);
};

error_t create_black_white_last_object_from_files(const char *previous_img,const char *current_img, uint8_t *black_white_img, const int updown, const int threshold){
    FILE* file_previouse_img;
    FILE* file_current_img;
    error_t ret=ESP_OK;
    file_previouse_img = fopen(previous_img, "r");
    if(file_previouse_img==NULL){
        ESP_LOGE(TAGFOTOLIB, "Error Open file %s",previous_img);
        return ESP_FAIL;
    }
    file_current_img = fopen(current_img, "r");
    if(file_previouse_img==NULL){
        ESP_LOGE(TAGFOTOLIB, "Error Open file %s",previous_img);
        return ESP_FAIL;
    }    
    int min_prev=255;
    int min_cur=255;
    int max_prev=0;
    int max_cur=0;
    int x=0;
    while(!feof(file_previouse_img) || !feof(file_current_img)){
        uint8_t buffer_previouse_img[1];
        uint8_t buffer_current_img[1];
        fread(buffer_previouse_img,sizeof(buffer_previouse_img),1,file_previouse_img);
        fread(buffer_current_img,sizeof(buffer_current_img),1,file_current_img);
        int pr;
        if(*buffer_previouse_img<min_prev)
            min_prev=*buffer_previouse_img;

        if(*buffer_previouse_img>max_prev)
            max_prev=*buffer_previouse_img;


        if(*buffer_previouse_img>threshold)
            pr=255;
        else
            pr=0;
        
        int cur;
        if(*buffer_current_img<min_cur)
            min_cur=*buffer_current_img;
        if(*buffer_current_img>max_cur)
            max_cur=*buffer_current_img;

        if(*buffer_current_img>threshold)
            cur=255;
        else
            cur=0;
    
        if(abs(*buffer_previouse_img-*buffer_current_img)> updown){
            black_white_img[x]=(uint8_t) 0;
        }else{
            black_white_img[x]=(uint8_t) 255;
        }
        x++;
    }
    fclose(file_previouse_img); 
    fclose(file_current_img); 
    ESP_LOGI(TAGFOTOLIB,"min prev=%d  cur=%d   max prev=%d  cur=%d ",min_prev,min_cur,max_prev, max_cur);
    return ret;
};

error_t crop_image(const uint8_t *src_image,uint8_t *dest_image,  Rec_change_img_t *rec_change_img, const size_t src_image_w,const size_t src_image_h, size_t *len_dest_image){
    error_t ret=ESP_OK;
    int index_dest_image=0;
    for (int y=rec_change_img->top ; y<=rec_change_img->bottom;y++){
        for(int x=rec_change_img->left; x<=rec_change_img->right; x++){
            dest_image[index_dest_image++]=src_image[(y*src_image_w)+x];
        }
    }
   
    *len_dest_image=index_dest_image-1;
    return ret;
};