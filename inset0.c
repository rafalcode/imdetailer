/* inset0: IMage Detailer Manual (Needs conft focus point file - 3 subimages, Top left. top right, bottom strip)
 * Takes from imdac3.c 
* an issue is how to retain aspect ratio
see asp0.c in cwhats. */
#include<math.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<cairo/cairo.h>
#include<cairo/cairo-pdf.h>
#include<cairo_jpg.h>

// we will save to a landscape sized PNG on which img2pdf can then run
// for an efficiently sized 
#define WPDF 842.
#define HPDF 595.
#define WFOC 200 // focus window width
#define HFOC 200 //height
#define THROWAWAYMEMSZ 64 // how many bytes to use for for strings which are usually going to be short.
// #define WPDF 800
// #define HPDF 600
#define LST 2 // line stroke width

#define DBG

#define GBUF 4
#define CONDREALLOC(x, b, c, a, t, a2, t2); \
    if((x)>=((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        (a2)=realloc((a2), (b)*sizeof(t2)); \
        for(i=((b)-(c));i<(b);++i) { \
            ((a)[i]) = NULL; \
            ((a2)[i]) = 0; \
        } \
    }

typedef enum /* imty_t: image type type: whether LANDSC (0) or PORTR (1) */
{
    LANDSC, PORTR
} imty_t;

typedef struct /* cf_t: colour float type */
{
    float r, g, b;
} cf_t;

typedef struct /* rpla_t: rectanglemetn placement type xy and top left pos, w and h width and height. */
{
    double x,y;
    double w,h;
    double lm, rm; // left marg and right margin
} rpla_t;

typedef struct /* larr_t line array struct */
{
    char **l;
    size_t *lz;
    int lbf;
    int asz;
} larr_t;

larr_t *slurplines(char *fn)
{
    FILE *stream;
    // char *line = NULL;
    size_t len = 0;
    int i;

    stream = fopen(fn, "r");
    if (stream == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    larr_t *la=malloc(sizeof(larr_t));
    la->lbf=GBUF;
    la->asz=0;
    la->l=malloc(la->lbf*sizeof(char*)); // does need this, yes, though later it won't be free'd
    la->lz=malloc(la->lbf*sizeof(size_t)); // does need this, yes, though later it won't be free'd
    for(i=0;i<la->lbf;++i) {
        la->lz[i]=0;
        la->l[i]=NULL;
    }

    while ((la->lz[la->asz] = getline(la->l+la->asz, &len, stream)) != -1) {
        // nread is number of returned characters
        CONDREALLOC(la->asz, la->lbf, GBUF, la->l, char*, la->lz, size_t);
        // printf("Retrieved line of length (returned val method): %zu\n", nread);
        // printf("Retrieved line of length (strlen method): %zu\n", strlen(line));
        // fwrite(line, nread, 1, stdout);
        if(la->l[la->asz][la->lz[la->asz]-1]=='\n') // check the newline
            la->l[la->asz][la->lz[la->asz]-1]='\0'; // smash the newline, of course you could save a byte and reallocate, etc. etc.
        la->asz++;
    }

    /* now to see if normalise works */
    for(i=la->asz;i<la->lbf;++i)
        free(la->l[i]);
    la->l=realloc(la->l, la->asz*sizeof(char*)); // normalize
    la->lz=realloc(la->lz, la->asz*sizeof(size_t)); // normalize

    /* Ok that's it */
    fclose(stream);
    return la;
}

void hc2f(cf_t *rgb, char *hstr) /* hex colour string to cf_t. Note: despite name it also handle decimetal rgb triples */
{
    size_t slen=strlen(hstr);
    char hxw[3]={'\0'};
    if((slen == 7) & (hstr[0] == '#')) { // it's hex then.
        hxw[0]=hstr[1]; hxw[1]=hstr[2];
        rgb->r=(float)strtol(hxw, NULL, 16)/255.;
        hxw[0]=hstr[3]; hxw[1]=hstr[4];
        rgb->g=(float)strtol(hxw, NULL, 16)/255.;
        hxw[0]=hstr[5]; hxw[1]=hstr[6];
        rgb->b=(float)strtol(hxw, NULL, 16)/255.;
        return;
    }

    char t=',';
	char *tk=strtok(hstr, &t);
	if(!tk) {
		printf("It appears this colour string is neither a hex nor a comma delimited rgb string.\n"); 
        exit(EXIT_FAILURE);
	}
    rgb->r=atof(tk)/255.;

	tk=strtok(NULL, &t);
    rgb->g=atof(tk)/255.;

	tk=strtok(NULL, &t);
    rgb->b=atof(tk)/255.;

    return;
}

void prtusage(char *progname)
{
    printf("this is %s, an imagedetailer program, it insets the detail within a reduced version of the image.\n", progname);
    printf("It requires one argument. A root file name, for which a \".jpg\" and \".conft\" need to be available.\n");
    printf("It should have one floating point number per line with 4 lines. the odd number lines are x coords. The evens, ycoords.\n");
    printf("the first pair are the centre of the region to be focused. The second, the centre of the inset region.\n");
}

int main (int argc, char *argv[])
{
    if(argc!=2) {
        prtusage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // sort out input jpg and conft from the argument (which is only a rootname).
    char *ijpg=calloc(THROWAWAYMEMSZ, sizeof(char));
    sprintf(ijpg, "%s.jpg", argv[1]);
    char *iconft=calloc(THROWAWAYMEMSZ, sizeof(char));
    sprintf(iconft, "%s.conft", argv[1]);
    printf("ijpg: %s iconft: %s\n", ijpg, iconft); 
    
    // now get hte two pointsfrom the iconft 
    larr_t *la=slurplines(iconft);
#ifdef DBG
    printf("how many text lines? i.e. asz:%i\n", la->asz); 
#endif
    if(la->asz!=4) {
        printf("Sorry but the conft file how many text lines? i.e. asz:%i\n", la->asz); 
        printf("Needs to be four only.\n"); 
        exit(EXIT_FAILURE);
    }

    //define output name, postfix it with imda.png!
    char *outname=calloc(THROWAWAYMEMSZ, sizeof(char));
    char *dot=strchr(argv[1], '.');
    sprintf(outname, "%.*s_ins0.png", (int)(dot-argv[1]), argv[1]);

    // OK let's take a look at input file
	struct stat ifsta;
	if(stat(ijpg, &ifsta) == -1) {
		fprintf(stderr,"Can't stat the input file %s", ijpg);
		exit(EXIT_FAILURE);
	}
	struct stat icsta;
	if(stat(iconft, &icsta) == -1) {
		fprintf(stderr,"Can't stat the input file %s", iconft);
		exit(EXIT_FAILURE);
    }

    // our surf0, this will be used as a source.
    cairo_surface_t *surf0 = cairo_image_surface_create_from_jpeg(ijpg);
    int iw = cairo_image_surface_get_width (surf0); // input jpg width
    int ih = cairo_image_surface_get_height (surf0); // input jpg height
    printf("orig input image w=%i,h=%i\n", iw, ih); 
    imty_t d1=LANDSC, d2=PORTR; // what is the first dimension. The default would be landscape, whereby widht is first.

    double howm0, aspa, aspb;
    aspa=(double)iw/ih;
    aspb=(double)WPDF/HPDF;
    printf("asps: %2.6f %2.6f\n",aspa, aspb); 
    if((aspb>aspa) & (aspa>1)) {
        printf("A landscape image: height of a needs biggest reduction. Width of image a will therefore be some way less than width of image b.\n");
        howm0=(double)ih/HPDF;
    } else if((aspb<aspa) & (aspa>1)) {
        // width of image a needs biggest reduction.
        printf("A landscape image: width of image a needs biggest reduction. Height of image a will therefore be some way less than width of image b.\n");
        howm0=(double)iw/WPDF;
    } else if((aspb<aspa) & (aspa<1)) {
        printf("A portrait image: width of image a needs biggest reduction. Height of image a will therefore be some way less than width of image b.\n");
        howm0=(double)iw/WPDF;
        d2=LANDSC;
        d1=PORTR;
    } else if((aspb>aspa) & (aspa<1)) {
        printf("A portrait image: height of a needs biggest reduction. Width of image a will therefore be some way less than width of image b.\n");
        howm0=(double)ih/HPDF;
        d2=LANDSC;
        d1=PORTR;
    }

    double owr[2];
    owr[d1]=iw/howm0;
    owr[d2]=ih/howm0;
    printf("new input image w=%2.4f,h=%2.4f (asp: %2.6f).\n", owr[d1], owr[d2], owr[d1]/owr[d2]);
    printf("canvar for this is w=%2.4f,h=%2.4f\n", WPDF, HPDF);

    // it might be necessary to decrease the most reduced amount one by one in the hope the aspect ratio can not be too distorted.    

    float mpoi[2] = {atof(la->l[0]), atof(la->l[1])};
    printf(".conft-file interest point on original:%2.2f,%2.2f\n", mpoi[0], mpoi[1]);
    float mpof[2] = {atof(la->l[2]), atof(la->l[3])};
    printf(".conft-file focus point on original:%2.2f,%2.2f\n", mpof[0], mpof[1]);

    cairo_surface_t *surf2 = cairo_image_surface_create(CAIRO_FORMAT_RGB24, owr[d1], owr[d2]);
    cairo_t *cr = cairo_create(surf2);
    double wrfac=1./howm0;
    printf("wrfac=%2.6f\n", wrfac); 
    cairo_scale(cr, wrfac, wrfac);
    cairo_set_source_surface (cr, surf0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    // now new one, so that scale is forgotten, he expects ...
    cr = cairo_create(surf2);
    rpla_t *r2=calloc(1, sizeof(rpla_t)); // so everything starts with zero.
    r2->x=wrfac*mpof[0]-WFOC/2.;
    r2->w=WFOC;
    r2->y=wrfac*mpof[1]-HFOC/2.;
    r2->h=HFOC;
    printf("cairo_rect r2: x=%2.2f, y=%2.2f, w=%2.2f, h=%2.2f\n", r2->x, r2->y, r2->w, r2->h);

    cairo_rectangle(cr, r2->x, r2->y, r2->w, r2->h);
    // DBG: just verify clip rectangle is OK.
    // cairo_set_source_rgb(cr, 1., 0, .5);
    // cairo_fill(cr);
    // cairo_set_line_width(cr, LST);
    // cairo_stroke_preserve(cr);
    cairo_clip(cr); // somehow, this means the rectangle is the only writeable part now.
    // OK only that clip is writeable

    float paintx=mpoi[0]-WFOC/2.;
    float painty=mpoi[1]-HFOC/2.;
    printf("paintstart x=%2.2f y=%2.2f\n", paintx, painty);
    cairo_set_source_surface(cr, surf0, r2->x-paintx, r2->y-painty);
    cairo_paint (cr);
    cairo_destroy (cr);
    
    // once again show the rectangle where poi is,
    cr = cairo_create(surf2);
    cairo_rectangle(cr, wrfac*paintx, wrfac*painty, wrfac*WFOC, wrfac*HFOC);
    cairo_set_source_rgb(cr, 1, 0, 0); // bright red!
    cairo_set_line_width(cr, LST);
    cairo_stroke_preserve(cr);

    cairo_rectangle(cr, r2->x, r2->y, r2->w, r2->h);
    // cairo_set_source_rgb(cr, 1, 0, 0); // bright red!
    cairo_set_line_width(cr, LST);
    cairo_stroke_preserve(cr);

    // connect the two rects? 
    cairo_move_to(cr, r2->x + r2->w, r2->y + r2->h);
    cairo_line_to(cr, wrfac*paintx, wrfac*painty);
    cairo_stroke(cr);


    cairo_destroy (cr);

    cairo_surface_write_to_png(surf2, outname);
    cairo_surface_destroy (surf0);
    cairo_surface_destroy (surf2);
    free(ijpg);
    free(iconft);

    return 0;
}
