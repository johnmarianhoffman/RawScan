#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "raw_scan.h"

#define DICOM_PREAMBLE ((const unsigned char *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x49\x43\x4d")
#define DICOM_PIXEL_DATA ((const unsigned char *)"\xe0\x7f\x10\x00")
#define DICOM_RAW_DATA_TAG_SIEMENS ((const unsigned char *)"\xe1\x7f\x10\x10")
#define DICOM_SCAN_INFO_TAG_SIEMENS ((const unsigned char *)"\x29\x00\x10\x10")
#define DICOM_XML_SCAN_INFO_TAG_SIEMENS ((const unsigned char *)"\x29\x00\x44\x10") // Not adequate for gettting data out of PTR files.  Revisit.
#define DICOM_MRN ((const unsigned char *)"\x10\x00\x20\x00")
#define DICOM_SOP_UID ((const unsigned char *)"\x08\x00\x18\x00")
#define DICOM_STUDY_DATE ((const unsigned char *)"\x08\x00\x20\x00")
#define L_DICOM_PREAMBLE 132
#define L_DICOM_TAG 4
#define L_VALUE_REP 2
#define L_OB_VR_PADDING 2

int make_prm_IMA_CTD(char * raw_file, char * prm_file){
    // Open our files
    FILE * f=fopen(raw_file,"rb");
    FILE * prm=fopen(prm_file,"w");

    //Read all of the data out of the scan info field
    void * scan_info;
    char * test;
    
    int b=extract_scan_data_from_CTD_IMA(f,&scan_info);
    if (b<0){
	printf("Something went wrong reading CTD/IMA to make PRM file.");
	exit(-1);
    }
    
    long offset;
    struct prm_info p;
    
    offset=scan_memory_for_offset(scan_info,(void*)"\nReconRangeBeginPos:",b,20,0L);
    if (offset<0){
	printf("Could not locate a ReconRangeBeginPos.");
    }
    else{
	sscanf(&scan_info[offset],"\nReconRangeBeginPos:\t%f\n",&p.recon_range_begin_pos);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nReconRangeEndPos:",b,18,0L);
    if (offset<0){
	printf("Could not locate a ReconRangeEndPos.");
    }
    else{
	sscanf(&scan_info[offset],"\nReconRangeEndPos:\t%f\n",&p.recon_range_end_pos);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nReadings:",b,10,0L);
    if (offset<0){
	printf("Could not locate a Readings.");
    }
    else{
	sscanf(&scan_info[offset],"\nReadings:\t%d\n",&p.n_readings);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nPitch:",b,7,0L);
    if (offset<0){
	printf("Could not locate a Pitch.");
    }
    else{
	sscanf(&scan_info[offset],"\nPitch:\t%f\n",&p.pitch);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nPitchFactor:",b,13,0L);
    if (offset<0){
	printf("Could not locate a PitchFactor.");
    }
    else{
	sscanf(&scan_info[offset],"\nPitchFactor:\t%f\n",&p.pitch_factor);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nNoOfSlicesDetector:",b,20,0L);
    if (offset<0){
	printf("Could not locate NoOfSlicesDetector.");
    }
    else{
	sscanf(&scan_info[offset],"\nNoOfSlicesDetector:\t%d\n",&p.no_of_slices_detector);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nImagesPerScan:",b,15,0L);
    if (offset<0){
	printf("Could not locate ImagesPerScan.");
    }
    else{
	sscanf(&scan_info[offset],"\nImagesPerScan:\t%d\n",&p.images_per_scan);
    }

    
    offset=scan_memory_for_offset(scan_info,(void*)"\nSliceDetector:",b,15,0L);
    if (offset<0){
	printf("Could not locate SliceDetector.");
    }
    else{
	sscanf(&scan_info[offset],"\nSliceDetector:\t%f\n",&p.slice_detector);
    }

    offset=scan_memory_for_offset(scan_info,(void*)"\nFoVHorLength:",b,14,0L);
    if (offset<0){
	printf("Could not locate FoVHorLength.");
    }
    else{
	sscanf(&scan_info[offset],"\nFoVHorLength:\t%f\n",&p.fov_hor_length);
    }

    // Split file from path:
    char * r_file;
    char * r_path;
    split_path_file(&r_path,&r_file,raw_file);
    // Split filename from extension
    char * r_f;
    char * r_e;
    int filetype;
    split_file_ext(&r_f,&r_e,r_file);

    size_t raw_data_start;
    
    if (!strcmp(r_e,"CTD")){
	filetype=2;
	raw_data_start=0;
    }
    else if (!strcmp(r_e,"ctd")){
	filetype=2;
	raw_data_start=0;
    }
    else if (!strcmp(r_e,"IMA")){
	filetype=3;
	// scan for raw_data offset
	raw_data_start=scan_file_for_offset(f,DICOM_RAW_DATA_TAG_SIEMENS,L_DICOM_TAG,0L);
	if (raw_data_start<0){
	    printf("File does not appear to be a Siemen's raw data file\n");
	    return -1;
	}

	// Add to offset: 4 (length of tag) + 2 (value identifier) + 6 (size identifier)
	raw_data_start+=12;
    }
    else if (!strcmp(r_e,"ima")){
	filetype=3;
	// scan for raw_data offset
	raw_data_start=scan_file_for_offset(f,DICOM_RAW_DATA_TAG_SIEMENS,L_DICOM_TAG,0L);
	if (raw_data_start<0){
	    printf("File does not appear to be a Siemen's raw data file\n");
	    return -1;
	}

	// Add to offset: 4 (length of tag) + 2 (value identifier) + 6 (size identifier)
	raw_data_start+=12;
	
    }

    // Write all of our data out to the requested PRM file
    fprintf(prm,"RawDataDir:\t%s\n",r_path);
    fprintf(prm,"RawDataFile:\t%s\n",r_file);
    fprintf(prm,"Nrows:\t%d\n",p.no_of_slices_detector);
    fprintf(prm,"CollSlicewidth:\t%.2f\n",p.slice_detector);
    fprintf(prm,"StartPos:\t%0.2f\n",p.recon_range_begin_pos);
    fprintf(prm,"EndPos:\t%0.2f\n",p.recon_range_end_pos);
    fprintf(prm,"SliceThickness:\t%0.2f\n",p.slice_detector);
    fprintf(prm,"AcqFOV:\t%0.2f\n",2*p.fov_hor_length);
    fprintf(prm,"ReconFOV:\t%0.2f\n",2*p.fov_hor_length);
    fprintf(prm,"ReconKernel:\t%d\n",20);
    fprintf(prm,"Readings:\t%d\n",p.n_readings);
    fprintf(prm,"Xorigin:\t0.0\n");
    fprintf(prm,"Yorigin:\t0.0\n");
    fprintf(prm,"Zffs:\t0\n");
    fprintf(prm,"Phiffs:\t0\n");
    fprintf(prm,"Scanner:\t2\n");
    fprintf(prm,"FileType:\t%d\n",filetype);
    fprintf(prm,"RawOffset:\t%lu\n",raw_data_start);
    
    free(scan_info);
    fclose(prm);
    fclose(f);
    return 0;
}

int extract_dicom_header_from_raw(FILE *fp,void ** header){
    
    fseek(fp,0L,SEEK_SET);
    size_t dicom_start=scan_file_for_offset(fp,DICOM_PREAMBLE,L_DICOM_PREAMBLE,0L);
    if (dicom_start<0){
	//printf("File does not appear to contain a dicom preamble\n");
	return -1;
    }
    
    size_t raw_data_start=scan_file_for_offset(fp,DICOM_RAW_DATA_TAG_SIEMENS,L_DICOM_TAG,dicom_start);
    if (raw_data_start<0){
	//printf("File does not appear to be a Siemen's raw data file\n");
	return -1;
    }

    *header=(void *)malloc(raw_data_start*sizeof(char));
    fseek(fp,dicom_start,SEEK_SET);
    fread(*header,sizeof(char),raw_data_start,fp);
    
    return raw_data_start;
}

int extract_dicom_header_from_any(FILE *fp,void ** header){
    
    fseek(fp,0L,SEEK_SET);
    size_t dicom_start=scan_file_for_offset(fp,DICOM_PREAMBLE,L_DICOM_PREAMBLE,0L);
    if (dicom_start<0){
	//printf("File does not appear to contain a dicom preamble\n");
	return -1;
    }

    // Look for pixel data first, then raw data
    size_t data_start=scan_file_for_offset(fp,DICOM_PIXEL_DATA,L_DICOM_TAG,dicom_start);
    if (data_start==-1){
	data_start=scan_file_for_offset(fp,DICOM_RAW_DATA_TAG_SIEMENS,L_DICOM_TAG,dicom_start);
	if (data_start==-1){
	    return -1;
	}
	
    }

    *header=(void *)malloc(data_start*sizeof(char));
    fseek(fp,dicom_start,SEEK_SET);
    fread(*header,sizeof(char),data_start,fp);
    
    return data_start;
}

int extract_scan_data_from_CTD_IMA(FILE * fp,void ** scan_info){
    fseek(fp,0L,SEEK_SET);
    size_t dicom_start=scan_file_for_offset(fp,DICOM_PREAMBLE,L_DICOM_PREAMBLE,0L);
    if (dicom_start<0){
	//printf("File does not appear to contain a dicom preamble\n");
	return -1;
    }
    
    size_t scan_info_start=scan_file_for_offset(fp,DICOM_SCAN_INFO_TAG_SIEMENS,L_DICOM_TAG,dicom_start);
    if (scan_info_start<0){
	//printf("File does not appear to be a Siemen's raw data file\n");
	return -1;
    }

    // We need to skip some bytes for the tag, value representation, then padding to get to the size
    size_t field_size_start=dicom_start+scan_info_start+L_DICOM_TAG+L_VALUE_REP+L_OB_VR_PADDING;
    // Seek to the field size and read in as 32-bit unsigned int
    fseek(fp,field_size_start,SEEK_SET);
    unsigned int scan_info_size;
    fread(&scan_info_size,sizeof(unsigned int),1,fp);

    // Now we allocate a buffer for the scan info, and fread it in
    *scan_info=(void *)malloc(scan_info_size*sizeof(char));
    fread(*scan_info,sizeof(char),scan_info_size,fp);

    return scan_info_size;
    
}

int extract_scan_data_from_PTR(FILE * fp,void ** scan_info){
    fseek(fp,0L,SEEK_SET);
    size_t dicom_start=scan_file_for_offset(fp,DICOM_PREAMBLE,L_DICOM_PREAMBLE,0L);
    if (dicom_start<0){
	printf("File does not appear to contain a dicom preamble\n");
	return -1;
    }
    
    size_t scan_info_start=scan_file_for_offset(fp,DICOM_XML_SCAN_INFO_TAG_SIEMENS,L_DICOM_TAG,dicom_start);
    if (scan_info_start<0){
	printf("File does not appear to be a Siemen's raw data file\n");
	return -1;
    }

    // We need to skip some bytes for the tag, value representation, then padding to get to the size
    size_t field_size_start=dicom_start+scan_info_start+L_DICOM_TAG+L_VALUE_REP+L_OB_VR_PADDING;
    // Seek to the field size and read in as 32-bit unsigned int
    fseek(fp,field_size_start,SEEK_SET);
    unsigned int scan_info_size;
    fread(&scan_info_size,sizeof(unsigned int),1,fp);

    // Now we allocate a buffer for the scan info, and fread it in
    *scan_info=(void *)malloc(scan_info_size*sizeof(char));
    fread(*scan_info,sizeof(char),scan_info_size,fp);

    return scan_info_size;
    
}


unsigned long scan_file_for_offset(FILE *fp,char* byte_string,size_t len,size_t start){
    unsigned long byte_offset;

    // Get file size in bytes (make sure to open with "rb")
    fseek(fp, 0L, SEEK_END);
    long f_sz = ftell(fp)-start;

    // Seek to the start location supplied
    fseek(fp, start, SEEK_SET);
    
    // Read the file into memory
    char * file=(char*)malloc(f_sz*sizeof(char));
    fread(file,sizeof(char),f_sz,fp);

    size_t b_sz=len;

    void * b1=file;
    void * b2=byte_string;
    
    void * ret;
    ret=memmem(b1,b2,f_sz,b_sz);
    if (ret==NULL){
	//printf("Could not find string inside of given file\n");
	byte_offset=-1L;
    }
    else{
	byte_offset=ret-b1;
    }
    
    free(file);
    return byte_offset;
}

// Essentially just a formatted call to memmem.  Returns byte offset between start and the first
// occurrance of needle in haystack.  Start is a byte offset from haystack.  Returns -1 if
// needle is not found in haystack.
unsigned long scan_memory_for_offset(void * haystack, void * needle,size_t s_h,size_t s_n,size_t start){

    void * res= memmem(haystack,needle,s_h,s_n);
    if (res==NULL){
	//printf("Did not find string in given memory region");
	return -1;
    }

    // Return the offset between "start" and 
    return res-haystack-start;
}

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD$");
#endif /* LIBC_SCCS and not lint */

#if !defined(_KERNEL) && !defined(_STANDALONE)
#include <assert.h>
#include <string.h>
#else
#include <lib/libkern/libkern.h>
#define _DIAGASSERT(x)  (void)0
#define NULL            ((char *)0)
#endif

/*
 * memmem() returns the location of the first occurence of data
 * pattern b2 of size len2 in memory block b1 of size len1 or
 * NULL if none is found.
 */
void *
memmem(const void *b1, const void *b2, size_t len1, size_t len2)
{
    /* Initialize search pointer */
    char *sp = (char *) b1;

    /* Initialize pattern pointer */
    char *pp = (char *) b2;

    /* Intialize end of search address space pointer */
    char *eos   = sp + len1 - len2;

    /* Sanity check */
    if(!(b1 && b2 && len1 && len2))
	return NULL;

    while (sp <= eos) {
	if (*sp == *pp)
	    if (memcmp(sp, pp, len2) == 0)
		return sp;

	sp++;
    }

    return NULL;
}

void split_path_file(char** p, char** f, char *pf) {
    char *slash = pf;
    char *next=NULL;
    while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
    if (pf != slash) slash++;
    *p = strndup(pf, slash - pf);
    *f = strdup(slash);
}

void split_file_ext(char** f, char** e, char *fe) {
    char *slash = fe;
    char *next=NULL;
    while ((next = strpbrk(slash + 1, "."))) slash = next;
    if (fe != slash) slash++;
    *f = strndup(fe, slash - fe);
    *e = strdup(slash);
}

/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
size_t strnlen (const char *string, size_t maxlen){
    const char *end = memchr (string, '\0', maxlen);
    return end ? end - string : maxlen;
}

char * strndup (const char *s, size_t n){
    size_t len = strnlen (s, n);
    char *new = malloc (len + 1);

    if (new == NULL)
	return NULL;

    new[len] = '\0';
    return memcpy (new, s, len);
}

char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}
