#ifndef raw_scan_h
#define raw_scan_h

struct prm_info{
    float recon_range_begin_pos;
    float recon_range_end_pos;
    int n_readings;
    float pitch;
    float pitch_factor;
    int images_per_scan;
    int no_of_slices_detector;
    float slice_detector;
    float fov_hor_length;
};

int make_prm_IMA_CTD(char *raw_file, char *prm_file);
int extract_dicom_header_from_raw(FILE * fp,void ** header);
int extract_dicom_header_from_any(FILE * fp,void ** header);
int extract_scan_data_from_CTD_IMA(FILE * fp, void ** scan_info);
int extract_Scan_data_from_PTR(FILE * FP, void ** scan_info);
unsigned long scan_file_for_offset(FILE* fp,char* byte_string,size_t len,size_t start);
unsigned long scan_memory_for_offset(void * haystack, void * needle,size_t s_h,size_t s_n,size_t start);
// random support functions
void * memmem(const void *b1,const void *b2,size_t len1,size_t len2);
void split_path_file(char ** p, char ** f, char *pf);
void split_file_ext(char** f, char** e, char *fe);
size_t strnlen (const char *string, size_t maxlen);
char * strndup(const char *S,size_t n);
char * strdup(const char *s);

#endif
