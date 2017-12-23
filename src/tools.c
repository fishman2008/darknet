#include <darknet.h>
#include "image.h"
#include "tools.h"


network network_init(const char *cfgfile, const char *weightfile, int threadsize)
{
    network net = parse_network_cfg(cfgfile);
    if (weightfile) {
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);

#ifdef NNPACK
    nnp_initialize();
    net.threadpool = pthreadpool_create(threadsize);
#endif

    return  net;
}

void network_release(network *net)
{
#ifdef NNPACK
    if(NULL != net)
    {
        pthreadpool_destroy(net->threadpool);
        nnp_deinitialize();
    }
#endif
}

image loadimage(char* filename, network *net)
{
#ifdef NNPACK
    image im = load_image_thread(filename, 0, 0, net->c, net->threadpool);
#else
    image im = load_image_color(filename, 0, 0);
#endif
    return im;
}

image convert2image(unsigned char *data, int w, int h, int c, int step)
{
    image im = make_image(w, h, c);
    int i, j, k;
    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    rgbgr_image(im);
    return im;
}

int detect_object(network * net, image im, float thresh, float hier_thresh,  float nms,  box *boxes, float **probs)
{
    struct timeval start, stop;
    gettimeofday(&start, 0);

#ifdef NNPACK
    image sized = letterbox_image_thread(im, net->w, net->h, net->threadpool);
#else
    image sized = letterbox_image(im, net->w, net->h);
#endif

    layer l = net->layers[net->n - 1];
    int j = 0;
    for (j = 0; j < l.w * l.h * l.n; ++j)
        probs[j] = calloc(l.classes + 1, sizeof (float *));

    float *X = sized.data;
   
    network_predict(*net, X); 

    //get_region_boxes(l, im.w, im.h, net->w, net->h, thresh, probs, boxes, 0, 0, hier_thresh, 1);
    if (nms) do_nms_obj(boxes, probs, l.w * l.h * l.n, l.classes, nms);

    free_image(sized);
    gettimeofday(&stop, 0);
    printf("Predicted in %ld ms.\n", (stop.tv_sec * 1000 + stop.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000));
    fflush( stdout);
    return  0;
}

void validate_detector(char *datacfg, char *cfgfile, char *weightfile, char *outpath, float thresh, float hier_thresh) {
    int j;
    list *options = read_data_cfg(datacfg);
    char *valid_images = option_find_str(options, "valid", "data/train.list");
    char *name_list = option_find_str(options, "names", "data/names.list");
    char **names = get_labels(name_list);

    double start = what_time_is_it_now();
    float nms = .3;
    list *plist = get_paths(valid_images);
    char **paths = (char **) list_to_array(plist);
    int m = plist->size;
    char buff[256];
    char *input = buff;
    char output[256] = {0};

    image **alphabet = load_alphabet();
    network net = network_init(cfgfile, weightfile, 4);
    layer l = net.layers[net.n - 1];
    box *boxes = calloc(l.w * l.h * l.n, sizeof (box));
    float **probs = calloc(l.w * l.h * l.n, sizeof (float *));


    int i = 0;
    for (i = 0; i < m; i++) {
        strncpy(input, paths[i], 256);
        strncpy(output, outpath, 256);
        char*file = rindex(input, '/') + 1;
        strcat(outpath, file);

        image im = loadimage(input, &net);
        memset(boxes, 0, l.w * l.h * l.n *sizeof (box));
        memset(probs, 0, l.w * l.h * l.n * sizeof (float *));

        detect_object(&net, im,  thresh, hier_thresh,  nms,  boxes, probs);

        //draw_detections(im, l.w * l.h * l.n, thresh, boxes, probs, names, alphabet, l.classes);
        save_image(im, output);

        free_image(im);
    }

    fprintf(stderr, "Total Detection Time: %f Seconds\n", (double) (time(0) - start));
    free(boxes);
    free_ptrs((void **) probs, l.w * l.h * l.n);
    network_release(&net);
}

void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh, char *outfile) {
    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    char **names = get_labels(name_list);

    srand(2222222);
    char buff[256];
    char *input = buff;
    float nms = .4;
    image **alphabet = load_alphabet();
    network net = network_init(cfgfile, weightfile, 4);
    layer l = net.layers[net.n - 1];
    box *boxes = calloc(l.w * l.h * l.n, sizeof (box));
    float **probs = calloc(l.w * l.h * l.n, sizeof (float *));


    while (1) {
        if (filename) {
            strncpy(input, filename, 256);
        } else {
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if (!input) return;
            strtok(input, "\n");
        }

        image im = loadimage(input, &net);
        memset(boxes, 0, l.w * l.h * l.n *sizeof (box));
        memset(probs, 0, l.w * l.h * l.n * sizeof (float *));
        detect_object(&net, im,  thresh, hier_thresh,  nms,  boxes, probs);
        //draw_detections(im, l.w * l.h * l.n, thresh, boxes, probs, names, alphabet, l.classes);
        if (outfile) {
            save_image(im, outfile);
        } else {
            save_image(im, "predictions");
        }

        free_image(im);
        if (filename) break;
    }

    free(boxes);
    free_ptrs((void **) probs, l.w * l.h * l.n);
    network_release(&net);
}
