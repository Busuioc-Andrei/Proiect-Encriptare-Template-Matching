#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct {
    unsigned char B;
    unsigned char G;
    unsigned char R;
}RGB;

typedef struct {
    char culoare;
    unsigned int x;
    unsigned int y;
    double corr;
}fereastra;

void xorshift(unsigned int seed,unsigned int n,unsigned int *randomArray){

    unsigned int r,k;
    r=seed;
    randomArray[0]=r;
    for(k=1;k<=n;k++){
        r=r^r<<13;
        r=r^r>>17;
        r=r^r<<5;
        randomArray[k]=r;
    }
}

void perm(unsigned int n,unsigned int *p,unsigned int *randomArray){
    unsigned int k,r,aux,i=1;
    for (k=0;k<n;k++)
        p[k]=k;
    for(k=n-1;k>=1;k--){
        r=randomArray[i]%(k+1);
        aux=p[r];
        p[r]=p[k];
        p[k]=aux;
        i++;
    }
}

void bmpTOarray(const char *numeIMG,unsigned char **header,RGB **V){
    FILE *fp= fopen(numeIMG,"rb");
    if (fp == NULL){
        printf("IMAGINEA NU A PUTUT FI DESCHISA");
        return;
    }
    *header=(unsigned char*)malloc(sizeof(unsigned char)*54);
    fread(*header,sizeof(unsigned char),54,fp);
    unsigned int height,width,padding=0;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];
    padding=(4-(3*width)%4);
    if(padding == 4)
        padding=0;
    puts("Image opened\n");
    printf("width x height = %u x %u\n",width,height);
    *V=(RGB*)calloc(width*height,sizeof(RGB));
    if(*V == 0){
        puts("Nu s-a putut aloca buffer pentru pixeli");
        fclose(fp);
        return;
    }
    int i,j;
    for(i=height-1;i>=0;i--){
        for(j=0;j<width;j++){
            fread(&(*V)[i*width+j],sizeof(unsigned char),3,fp);
            }
        if(padding != 0)
            fseek(fp,padding,SEEK_CUR);
    }

    fclose(fp);

}

void arrayTObmp(char *numeIMG,unsigned char **header,RGB **V){
    FILE *fp= fopen(numeIMG,"wb");
    if (fp == NULL){
        printf("IMAGINEA NU A PUTUT FI DESCHISA");
        return;
    }

    fwrite(*header,sizeof(unsigned char),54,fp);
    unsigned int height,width,padding=0;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];
    padding=(4-(3*width)%4);
    if(padding == 4)
        padding=0;
    //printf("width x height = %u x %u\n",width,height);
    int i,j,zero=0;

    for(i=height-1;i>=0;i--){
        for(j=0;j<width;j++){
            fwrite(&(*V)[i*width+j],sizeof(unsigned char),3,fp);
            }
        if(padding != 0)
            fwrite(&zero,padding,1,fp);
    }
    puts("Image saved\n");
    fclose(fp);


}

unsigned int RGBtoINT(RGB a){
    unsigned int x=0;
    x=x|a.R;
    x=x<<8;
    x=x|a.G;
    x=x<<8;
    x=x|a.B;
    return x;
}

RGB INTtoRGB(unsigned int a){
    RGB x;
    x.B=(unsigned char)a;
    a=a>>8;
    x.G=(unsigned char)a;
    a=a>>8;
    x.R=(unsigned char)a;
    return x;
}


void criptare(unsigned char **header,RGB **V,const char *cheie){
    FILE *fp= fopen(cheie,"r");
    if (fp == NULL){
        printf("CHEIA NU A PUTUT FI DESCHISA\n");
        return;
    }
    unsigned int R0,SV;
    fscanf(fp,"%u %u",&R0,&SV);

    unsigned int height,width;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];

    unsigned int *r=malloc((2*width*height)*sizeof(unsigned int));
    xorshift(R0,2*width*height-1,r);
    unsigned int k,j;

    unsigned int *p=calloc(width*height,sizeof(int));
    perm(width*height,p,r);

    RGB *P=(RGB*)calloc(width*height,sizeof(RGB));
    for(j=0;j<width*height;j++)
        P[j]=(*V)[j];

    for(k=0;k<width*height;k++){
       (*V)[p[k]]=P[k];
    }


    int auxI2,auxI=RGBtoINT((*V)[0]);
    auxI2=(SV)^( auxI )^(r[width*height]);
    (*V)[0]=INTtoRGB(auxI2);
    for(k=1;k<width*height;k++){
        auxI=RGBtoINT((*V)[k-1]);
        auxI2=RGBtoINT((*V)[k]);
        auxI2 =( auxI )^( auxI2 )^( r[width*height+k] );
        (*V)[k] = INTtoRGB(auxI2);
    }
    free(r);
    free(p);
    free(P);
    puts("Imagine criptata");
}


void decriptare(unsigned char **header,RGB **V,const char *cheie){
    FILE *fp= fopen(cheie,"r");
    if (fp == NULL){
        printf("CHEIA NU A PUTUT FI DESCHISA\n");
        return;
    }
    unsigned int R0,SV;
    fscanf(fp,"%u %u",&R0,&SV);

    unsigned int height,width;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];

    unsigned int *r=malloc((2*width*height)*sizeof(unsigned int));
    xorshift(R0,2*width*height-1,r);
    unsigned int k,j;

    RGB *C=(RGB*)calloc(width*height,sizeof(RGB));
    for(j=0;j<width*height;j++)
        C[j]=(*V)[j];

    int auxI2,auxI=RGBtoINT((*V)[0]);
    auxI2=(SV)^( auxI )^(r[width*height]);
    (*V)[0]=INTtoRGB(auxI2);
    C[0]=(*V)[0];
    for(k=1;k<width*height;k++){
        auxI=RGBtoINT(C[k-1]);
        auxI2=RGBtoINT((*V)[k]);
        auxI2 =( auxI )^( auxI2 )^( r[width*height+k] );
        (*V)[k] = INTtoRGB(auxI2);
    }

    unsigned int *p=calloc(width*height,sizeof(int));
    unsigned int *pi=calloc(width*height,sizeof(int));
    perm(width*height,p,r);
    for(k=0;k<width*height;k++){
        pi[p[k]]=k;
    }

    RGB *P=(RGB*)calloc(width*height,sizeof(RGB));
    for(j=0;j<width*height;j++)
        P[j]=(*V)[j];

    for(k=0;k<width*height;k++){
       (*V)[pi[k]]=P[k];
    }


    free(r);
    free(p);
    free(pi);
    free(C);
    free(P);
    puts("Imagine decriptata");

}

void chiTest (unsigned char **header,RGB **V){
    unsigned int height,width;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];

    int i;
    double f,chisqR=0,chisqG=0,chisqB=0;
    double fBar=(width*height)/256;
    int *fR=calloc(256,sizeof(int));
    int *fG=calloc(256,sizeof(int));
    int *fB=calloc(256,sizeof(int));
    for(i=0;i<width*height;i++){
        fR[(*V)[i].R]++;
        fG[(*V)[i].G]++;
        fB[(*V)[i].B]++;
    }
    for(i=0;i<=255;i++){
        f=fR[i];
        chisqR=chisqR+ ((f-fBar)*(f-fBar))/fBar;
    }
    printf("%.2f\n",chisqR);

    for(i=0;i<=255;i++){
        f=fG[i];
        chisqG=chisqG+ ((f-fBar)*(f-fBar))/fBar;
    }
    printf("%.2f\n",chisqG);

    for(i=0;i<=255;i++){
        f=fB[i];
        chisqB=chisqB+ ((f-fBar)*(f-fBar))/fBar;
    }
    printf("%.2f\n",chisqB);
    free(fR);
    free(fG);
    free(fB);
}
/////////////////////////////////PART 2

void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
   FILE *fin, *fout;
   unsigned int dim_img, latime_img, inaltime_img;
   unsigned char pRGB[3], header[54], aux;

   printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);

   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 2, SEEK_SET);
   fread(&dim_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

   fseek(fin, 18, SEEK_SET);
   fread(&latime_img, sizeof(unsigned int), 1, fin);
   fread(&inaltime_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

   //copiaza octet cu octet imaginea initiala in cea noua
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);

	//calculam padding-ul pentru o linie
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    printf("padding = %d \n",padding);

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//citesc culorile pixelului
			fread(pRGB, 3, 1, fout);
			//fac conversia in pixel gri
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}



void bmpTOmatrix(const char *numeIMG,unsigned char **header,RGB ***V){
    FILE *fp= fopen(numeIMG,"rb");
    if (fp == NULL){
        printf("IMAGINEA NU A PUTUT FI DESCHISA");
        return;
    }
    *header=(unsigned char*)malloc(sizeof(unsigned char)*54);
    fread(*header,sizeof(unsigned char),54,fp);
    unsigned int height,width,padding=0;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];
    padding=(4-(3*width)%4);
    if(padding == 4)
        padding=0;
    puts("Image opened\n");
    printf("width x height = %u x %u\n",width,height);
    *V=(RGB**)calloc(height,sizeof(RGB*));

    int i;
    for(i=0;i<height;i++)
        (*V)[i]=(RGB*)calloc(width,sizeof(RGB));

    int j;


    for(i=height-1;i>=0;i--){
        for(j=0;j<width;j++){
            fread(&(*V)[i][j],sizeof(unsigned char),3,fp);
            }
        if(padding != 0)
            fseek(fp,padding,SEEK_CUR);
    }

    fclose(fp);

}


void matrixTObmp(char *numeIMG,unsigned char **header,RGB ***V){
    FILE *fp= fopen(numeIMG,"wb");
    if (fp == NULL){
        printf("IMAGINEA NU A PUTUT FI DESCHISA");
        return;
    }

    fwrite(*header,sizeof(unsigned char),54,fp);
    unsigned int height,width,padding=0;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];
    padding=(4-(3*width)%4);
    if(padding == 4)
        padding=0;
    //printf("width x height = %u x %u\n",width,height);
    int i,j,zero=0;

    for(i=height-1;i>=0;i--){
        for(j=0;j<width;j++){
            fwrite(&(*V)[i][j],sizeof(unsigned char),3,fp);
            }
        if(padding != 0)
            fwrite(&zero,padding,1,fp);
    }
    puts("Image saved\n");
    fclose(fp);


}


void contur(RGB ***M,fereastra F,RGB C){
    int i;
    for (i=0;i<15;i++){
        (*M)[F.x+i][F.y+10]=C;
        (*M)[F.x+i][F.y]=C;
    }
    for (i=0;i<11;i++){
        (*M)[F.x+14][F.y+i]=C;
        (*M)[F.x][F.y+i]=C;
    }
}


double xBar(RGB **S,int x,int y){
    int i,j;
    double med=0;
    for(i=0;i<15;i++){
        for(j=0;j<11;j++){
            med=med+S[x+i][y+j].R;
        }
    }
    med=med/165;
    return med;
}


double xSig(RGB **S,int x,int y,double sBar){
    int i,j;
    double sum=0;
    for(i=0;i<15;i++){
        for(j=0;j<11;j++){
            sum=sum + (S[x+i][y+j].R - sBar) * (S[x+i][y+j].R - sBar) ;
        }
    }
    sum = sqrt(sum/164);
    return sum;
}

double xDif(RGB **S,RGB **F,int x,int y,double fBar,double sBar){
    int i,j;
    double sum=0;
    for(i=0;i<15;i++){
        for(j=0;j<11;j++){
            sum=sum + (F[x+i][y+j].R - fBar) * (S[i][j].R - sBar) ;
        }
    }
    return sum;
}


int cmp(const void *a,const void *b){
    fereastra *FA=(fereastra*)a;
    fereastra *FB=(fereastra*)b;
    if(FA->corr >= FB->corr)
        return -1;
    else
        return 1;
}


fereastra * templateMatching(unsigned char **header,RGB ***M,RGB ***S,double ps){
    unsigned int height,width;
    width=*(unsigned int*)&(*header)[18];
    height=*(unsigned int*)&(*header)[22];
    int n = 165;
    int i,j,k = 1;
    fereastra *D = malloc(sizeof(fereastra));

    D[0].y = 0;
    D[0].corr = 2;
    double fBar = 0,sBar = 0;
    double fSig = 0,sSig = 0;
    double dif = 0,corr = 0;
    sBar=xBar((*S),0,0);
    sSig=xSig((*S),0,0,sBar);
    for(i=0;i<height-15;i++){
        for(j=0;j<width-11;j++){
            fBar=xBar((*M),i,j);
            fSig=xSig((*M),i,j,fBar);
            dif=xDif((*S),(*M),i,j,fBar,sBar);
            corr=dif/(n*fSig*sSig);
            if(corr >= ps){
                D[0].x = k; ///CONTOR
                D = realloc(D,(k+1)*sizeof(fereastra));
                D[k].x=i;
                D[k].y=j;
                D[k].corr=corr;
                k++;
            }
        }
    }
    return D;
}

int suprapunere(fereastra A,fereastra B){
    double inters = 0;
    double supraP = 0;
    int i,j,I,J;
    int X,Y;
    for(i=0;i<15;i++){
        for(j=0;j<11;j++){
            X=A.x+i;
            Y=A.y+j;
               for(I=0;I<15;I++)
                    for(J=0;J<11;J++)
                         if( (X == B.x+ I) && (Y == B.y + J) ){
                            inters++;
                            I=15;
                            J=11;
                            }
        }
    }
    supraP = inters/(330 - inters);
    if (supraP > 0.2)
        return 1;
    else
        return 0;
}

void elimNONmax(fereastra **D){
    int size=(*D)[0].x;
    int i,j;
    for (i=1;i<size-1;i++){
        for(j=i+1;j<size;j++){
            if( ( (*D)[i].corr !=0 ) &&( (*D)[j].corr !=0 ) )
                if(suprapunere( (*D)[i] , (*D)[j] ) )
                    (*D)[j].corr = 0;
        }
    }
    int k = 1;
    for (i=1;i<size;i++){
        if( (*D)[i].corr != 0){
            (*D)[k]=(*D)[i];
            k++;
        }
    }
    (*D)[0].x = k-1;
}

void deallocate_mem(int*** arr, int n){
    for (int i = 0; i < n; i++)
        free((*arr)[i]);
    free(*arr);
}

int main()
{
    char numeIMG[101],numeIMG2[101],numeIMG3[101],cheie[101];

    puts("Encriptare:\n");

    printf("Numele fisierului care contine imaginea sursa: ");
    fgets(numeIMG, 101, stdin);
    numeIMG[strlen(numeIMG) - 1] = '\0';

    printf("Numele fisierului care contine imaginea encriptata: ");
    fgets(numeIMG2, 101, stdin);
    numeIMG2[strlen(numeIMG2) - 1] = '\0';

    printf("Numele fisierului care contine imaginea decriptata: ");
    fgets(numeIMG3, 101, stdin);
    numeIMG3[strlen(numeIMG3) - 1] = '\0';

    printf("Numele fisierului care contine cheia secreta: ");
    fgets(cheie, 101, stdin);
    cheie[strlen(cheie) - 1] = '\0';

    unsigned char *header=0;
    RGB *V = 0;
    bmpTOarray(numeIMG,&header, &V);
    chiTest(&header,&V);
    puts("\n");

    criptare(&header,&V,cheie);
    chiTest(&header,&V);

    arrayTObmp(numeIMG2,&header,&V);
    decriptare(&header,&V,cheie);

    arrayTObmp(numeIMG3,&header,&V);
    free(V);
    free(header);

    ////////PART 2
    puts("Template matching:\n");

    char nume_img_sursa[101];
    printf("Numele fisierului care contine imaginea sursa pentru template-matching: ");
    fgets(nume_img_sursa, 101, stdin);
    nume_img_sursa[strlen(nume_img_sursa) - 1] = '\0';

    char nume_output[101];
    printf("Numele fisierului care contine imaginea output: ");
    fgets(nume_output, 101, stdin);
    nume_output[strlen(nume_output) - 1] = '\0';

    char nume_sabloane[101];
    printf("Numele fisierului care contine numele sabloanelor: ");
    fgets(nume_sabloane, 101, stdin);
    nume_sabloane[strlen(nume_sabloane) - 1] = '\0';

	char nume_img_grayscale[] = "test_grayscale.bmp";
	unsigned char *headerColor = 0;
	unsigned char *headerTest = 0;
	unsigned char *throwAwayHeader = 0;
	RGB *M = 0;
	RGB *N = 0;
	RGB *S = 0;

	bmpTOmatrix(nume_img_sursa,&headerColor,&N);
	grayscale_image(nume_img_sursa, nume_img_grayscale);
	bmpTOmatrix(nume_img_grayscale,&headerTest,&M);

	RGB C[]={
	{0,0,255},
	{0,255,255},
	{0,255,0},
	{255,255,0},
	{255,0,255},
	{255,0,0},
	{192,192,192},
    {0,140,255},
    {128,0,128},
    {0,0,128}
};

	int i;
	fereastra *bigD = 0;
	int bigDsize = 0;
    FILE *fp = fopen(nume_sabloane,"r");

	for (i=0;i<=9;i++){
        if(fp == NULL){
            printf("FISIERUL CU NUMELE SABLOANELOR NU A PUTUT FI DESCHIS\n");
            return;
        }
        char numeSablon[101],numeSablonGS[101];
        fgets(numeSablon, 101, fp);
        strcpy(numeSablonGS,numeSablon);
        numeSablonGS[strlen(numeSablonGS) - 8] = 'G';
        numeSablonGS[strlen(numeSablonGS) - 7] = 'S';
        numeSablonGS[strlen(numeSablonGS) - 1] = '\0';
        numeSablon[strlen(numeSablon) - 1] = '\0';

        FILE *fsp = fopen(numeSablon,"rb");
        if(fsp == NULL){
            printf("FISIERUL %s NU A PUTUT FI DESCHIS\n",numeSablon);
            return;
        }
        grayscale_image(numeSablon,numeSablonGS);
        bmpTOmatrix(numeSablonGS,&throwAwayHeader,&S);
        fereastra *D=templateMatching(&headerTest,&M,&S,0.5);
        int size = D[0].x;
        bigD = realloc(bigD,(size+bigDsize+1)*sizeof(fereastra));
        int j;
        for(j=bigDsize+1;j<=bigDsize+size;j++){
            bigD[j]=D[j-bigDsize];
            bigD[j].culoare=i;
            }
        bigDsize = bigDsize + size;

        free(D);
        deallocate_mem(&S,15);
        fclose(fsp);
	}
	fclose(fp);

	bigD[0].x=bigDsize;
	bigD[0].y=0;
	bigD[0].corr=2;

	printf("Detectii totale:%d\n",bigDsize);
    qsort(bigD,bigDsize+1,sizeof(fereastra),cmp);
	elimNONmax(&bigD);
	bigDsize=bigD[0].x;
	printf("Detectii ramase:%d\n",bigD[0].x);
	for(i=1;i<=bigDsize;i++){
        contur(&N,bigD[i],C[bigD[i].culoare]);
    }

    matrixTObmp(nume_output,&headerColor,&N);

    free(bigD);
    int height=*(unsigned int*)&headerTest[22];
    deallocate_mem(&M,height);
    height=*(unsigned int*)&headerColor[22];
    deallocate_mem(&N,height);
    free(headerTest);
    free(headerColor);
    free(throwAwayHeader);
    return 0;
}
