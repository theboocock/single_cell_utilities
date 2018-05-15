/*
 * Copyright (c) 2018 Boocock James <james.boocock@otago.ac.nz>
 * Author: Boocock James <james.boocock@otago.ac.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <htslib/hts.h>
#include <htslib/sam.h>

void usage(const char* prog){
    fprintf(stderr, "Usage: %s -i <in.bam> -o <out.prefix> -t <tag_to_de_multiplex>\n", prog);
}

int main(int argc, char* argv[])
{
    htsFile *in = NULL;
    htsFile *out = NULL;
    char *in_name = "-";
    char *out_prefix= "output/";
    char *ref_name = NULL;
    char *ref_seq = NULL;
    char modew[8] = "wb";
    char *tag_id = NULL;
    char *tag_value = NULL;
    char *tag_tmp = NULL;
    char out_name[255];
    bam_hdr_t *hdr = NULL;
    bam1_t *rec = NULL;
    uint8_t *tag = NULL;
    int c,res;
    int current_file = 0;
    int file_exists = 0;
    while((c = getopt(argc, argv, "t:ho:i:")) >=0){
        switch(c){
            case 'o': out_prefix = optarg; break;
            case 'i': in_name = optarg; break;
            case 't': tag_id = optarg; break;
            case 'h': usage(argv[0]); return EXIT_SUCCESS;
            default: usage(argv[0]); return EXIT_FAILURE;
        }
    }
    rec = bam_init1();
    if (!rec) {
        perror(NULL);
        goto fail;
    }
    fprintf(stderr, "Processing tag sorted SAM file into individual files\n");
    in = sam_open(in_name,"r"); 
    if (!in)  {
        fprintf(stderr, "Couldn't open %s : %s\n", in_name, strerror(errno));
        goto fail;
    }
    hdr = sam_hdr_read(in);
    assert(hdr);
    while((res = sam_read1(in, hdr, rec)) > 0){
        tag = bam_aux_get(rec, tag_id);
        if (tag == NULL){
            fprintf(stderr, "TAG: %s not present in read\n",tag_id);
        }else{
            tag_tmp = bam_aux2Z(tag);
            // 0 represents no file currently, 1 represents that we have a file running.
            if(tag_value == NULL || strcmp(tag_tmp,tag_value) != 0){
                if (out != NULL){
                    hts_close(out);
                }
                snprintf(out_name, sizeof out_name ,"%s%s%s%s",out_prefix,"/",tag_tmp,".bam");
                out = hts_open(out_name, modew);
                if (!out) {
                    fprintf(stderr, "Couldn't open %s : %s\n", out_name, strerror(errno));
                    goto fail;
                }
                 if (sam_hdr_write(out, hdr) < 0) {
                    fprintf(stderr, "Couldn't write header to %s : %s\n",
                    out_name, strerror(errno));
                    goto fail;
                }
            }
            tag_value = strdup(tag_tmp);
            if(sam_write1(out,hdr,rec) < 0){
                fprintf(stderr, "Error writing to %s\n", out_name);
                goto fail;
            }
        }
    }
    if (out != NULL){
        hts_close(out);
    }
    fprintf(stderr, "Finished processing tag sorted SAM file into individual files\n");
    //bam1_core_t *d = &rec->core;
    //fprintf(stderr, "recorde%d\n", d);
    //fprintf(stderr, "recorde%s\n", d->pos);

    return EXIT_SUCCESS;
     fail:
        if (hdr) bam_hdr_destroy(hdr);
        if (rec) bam_destroy1(rec);
        free(ref_seq);
        return EXIT_FAILURE;
}
