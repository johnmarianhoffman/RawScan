#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raw_scan.h"

#define DICOM_SOP_UID ((const unsigned char *)"\x20\x00\x0d\x00")
#define DICOM_MRN ((const unsigned char *)"\x10\x00\x20\x00")
#define DICOM_STUDY_DATE ((const unsigned char *)"\x08\x00\x20\x00")
#define L_DICOM_PREAMBLE 132
#define L_DICOM_TAG 4
#define L_VALUE_REP 2
#define L_OB_VR_PADDING 2

struct flags{
    int write_to_disk;
    int header;
    int sop_uid;
    int mrn;
    int study_date;
    int quiet;
    int raw;
    int prm;
    int have_raw;
    int have_out;
};

void usage(){
    printf("\n");
    printf("usage: raw_scan [options] raw_file [out_file]\n\n");
    printf("    Options:\n");
    printf("          -h: extract and write header to disk. Don't use with any other options.\n");
    printf("        -out: write extracted data to disk.\n");
    printf("        -uid: extract and print study instance uid to stdout.\n");
    printf("        -mrn: extract and print mrn to stdout.\n");
    printf("         -sd: extract and print study date to stdout.\n");
    printf("          -q: quiet mode, only output uid, mrn, sd if found, otherwise no messages. Useful for batch scripting.\n");
    printf("          -r: search for dicom header using Siemen's raw data tag, otherwise we use the pixeldata tag first, then raw data tag.\n");
    printf("        -prm: generate a .prm file used for reconstruction. Don't use with any other options\n");
    printf("\n");
    printf("Copyright John Hoffman 2015\n\n");
    exit(0);
}

int main(int argc,char** argv){

    if (argc<2)
	usage();
    
    struct flags f;
    memset(&f,0,sizeof(struct flags));

    char raw_file[4096+255]={0};
    char out_file[4096+255]={0};
    
    // Process command line arguments
    for (int i=1;i<argc;i++){

	// -h: extract and write header to disk
	if (strcmp(argv[i],"-h")==0){
	    f.header=1;
	}
	
	// -s: output extract header to disk
	if (strcmp(argv[i],"-out")==0){
	    f.write_to_disk=1;
	}

	// -uid: extract the sop uid
	else if (strcmp(argv[i],"-uid")==0){
	    f.sop_uid=1;
	}

	// -mrn: extract patient ID field
	else if (strcmp(argv[i],"-mrn")==0){
	    f.mrn=1;
	}

	// -sd: extract study date field
	else if (strcmp(argv[i],"-sd")==0){
	    f.study_date=1;
	}

	// -q: quiet mode
	else if (strcmp(argv[i],"-q")==0){
	    f.quiet=1;
	}

	// -r: raw only
	else if (strcmp(argv[i],"-r")==0){
	    f.raw=1;
	}

	// -prm: generate prm file
	else if (strcmp(argv[i],"-prm")==0){
	    f.prm=1;
	}

	else if (strcmp(argv[i],"--help")==0){
	    usage();
	}
	
	else{
	    // First non-option argument is raw data file
	    if (!f.have_raw){
		strcpy(raw_file,argv[i]);
		f.have_raw=1;
		continue;
	    }
	    // Second non-option argument is output file
	    if (!f.have_out&&(f.write_to_disk||f.prm||f.header)){
		strcpy(out_file,argv[i]);
		f.have_out=1;
	    }
	}
    }

    // Extract the DICOM header no matter what
    /* errno=0; */
    /* FILE * rf=fopen(raw_file,"rb"); */
    /* if (rf==NULL){ */
    /* 	fprintf (stderr, "%s: Couldn't open file %s; %s\n", */
    /* 		 argv[0], raw_file, strerror(errno)); */
    /* 	exit (EXIT_FAILURE); */
    /* } */
	
    /* void * header; */
    /* size_t bytes; */
    /* if (f.raw){ */
    /* 	bytes=extract_dicom_header_from_raw(rf,&header); */
    /* } */
    /* else{ */
    /* 	bytes=extract_dicom_header_from_any(rf,&header); */
    /* } */
    /* fclose(rf); */

    /* if (bytes==-1L){ */
    /* 	// Exit if we find no dicom header. */
    /* 	if (!f.quiet) */
    /* 	    printf("DICOM data not found\n"); */
    /* 	exit(0); */
    /* } */

    /* FILE * h; */
    
    /* if (f.header){ */
    /* 	h=fopen(out_file,"w"); */
    /* 	fwrite(header,sizeof(char),bytes,h); */
    /* 	fclose(h); */
    /* 	exit(0); */
    /* } */
    
    /* unsigned long offset; */
    /* size_t field_size_start; */
    /* size_t field_size; */

    /* char * uid; */
    /* char * study_date; */
    /* char * mrn; */

    /* if (f.write_to_disk) */
    /* 	h=fopen(out_file,"w"); */
    
    /* if (f.sop_uid){ */
    /* 	offset=scan_memory_for_offset(header,DICOM_SOP_UID,bytes,4,0); */

    /* 	if (offset<0){ */
    /* 	    if (!f.quiet) */
    /* 		printf("UID not found\n"); */
    /* 	} */
    /* 	else{ */
    /* 	    field_size_start=offset+L_DICOM_TAG+L_VALUE_REP; */
    /* 	    // Grab the size */
    /* 	    memcpy(&field_size,&header[field_size_start],sizeof(char)*2); */
    /* 	    // Grab the data */
    /* 	    uid=(char *)calloc(field_size,sizeof(char)); */
    /* 	    memcpy(uid,&header[field_size_start+2],sizeof(char)*field_size); */
    /* 	    printf("StudyInstanceUID: %s\n",uid); */

	    if (f.write_to_disk)
		fprintf(h,"StudyInstanceUID: %s\n",uid);
	}
    }

    if (f.study_date){
	offset=scan_memory_for_offset(header,DICOM_STUDY_DATE,bytes,4,0);

	if (offset<0){
	    if (!f.quiet)
		printf("Study date not found\n");
	}
	else{
	    field_size_start=offset+L_DICOM_TAG+L_VALUE_REP;
	    // Grab the size
	    memcpy(&field_size,&header[field_size_start],sizeof(char)*2);
	    // Grab the data
	    study_date=(char *)calloc(field_size,sizeof(char));
	    memcpy(study_date,&header[field_size_start+2],sizeof(char)*field_size);
	    printf("Study Date: %s\n",study_date);

	    if (f.write_to_disk)
		fprintf(h,"Study Date: %s\n",study_date);
	}
    }
    
    if (f.mrn){
	offset=scan_memory_for_offset(header,DICOM_MRN,bytes,4,0);

	if (offset<0){
	    if (!f.quiet)
		printf("MRN not found\n");
	}
	else{
	    field_size_start=offset+L_DICOM_TAG+L_VALUE_REP;
	    // Grab the size
	    memcpy(&field_size,&header[field_size_start],sizeof(char)*2);
	    // Grab the data
	    mrn=(char *)calloc(field_size,sizeof(char));
	    memcpy(mrn,&header[field_size_start+2],sizeof(char)*field_size);
	    printf("MRN: %s\n",mrn);

	    if (f.write_to_disk)
		fprintf(h,"MRN: %s\n",mrn);
	}
    }

    if (f.prm){
	make_prm_IMA_CTD(raw_file,out_file);
    }

    if (f.write_to_disk)
	fclose(h);

    return 0;
}
