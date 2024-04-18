
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define df_MAX_BUFFER   50
#define df_MONTH        12

// PIS-16S 로직에서 버퍼 사이즈 보다 큰 경우가 많아서 설정
#define df_MAX_CONTENTS_SIZE    10000

static char* ContentsBuffer;
static char StartPoint[] = "00 :";

static char TargetFile[2][20] = {
    "text.txt",
    "text2.txt"
};
static char MakeFileName[2][20]= {
    "Make.txt",
    "Make2.txt"
};
static char Mon[12][10] = {
    "Jan", 
    "Fab", 
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};



int GotoStart(char**, const char* const);
int CheckMonth(char**);
void JobForPIU_16S(FILE*, char*, const char* const);


int main(int argc, char* argv[])
{

    int iInput;
    printf("Enter 0 for PIU-16SC(10G) or 1 for PIU-16S(1G) : ");
    scanf("%d", &iInput);


    

    FILE* fp;
    fp = fopen(TargetFile[iInput], "r");
    if(!fp)
    {
        printf("fail to open file\n");
        return -1;
    }


    fseek(fp, 0, SEEK_END);
    int iSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("File Size = %d\n", iSize);



    char* pFileContents = (char*)malloc(iSize + 1);
    memset(pFileContents, 0, iSize + 1);

    int iFreadReturn = fread(pFileContents, 1, iSize, fp);
    printf("fread Return = %d\n", iFreadReturn);

    if(iFreadReturn != iSize)
    {
        printf("Error fread!\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);






    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      Make Data    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fp = fopen(MakeFileName[iInput], "a+");

    char* ptr = pFileContents;
    const char* const pEndPoint = pFileContents + iSize;
    char Buffer[df_MAX_BUFFER];

    fprintf(fp, "-------------------------------------------------------------------------------------------------------------\n\n");


    if(iInput == 0)
    {



        while(ptr != pEndPoint)
        {
            if(!GotoStart(&ptr, pEndPoint))
                break;

            int iStop = 0;
            while(!iStop)
            {
                if(*ptr != ' ')
                    fputc(' ', fp);

                memset(Buffer, 0, df_MAX_BUFFER);
                memcpy(Buffer, ptr, 48);

                fprintf(fp, "%s\n", Buffer);
                ptr += 48;

                while(*ptr != ':')
                {
                    if(*ptr == 'A')
                    {
                        if(CheckMonth(&ptr))
                        {
                            iStop = 1;
                            fprintf(fp, "\n\n\n\n");

                            break;
                        }
                    }
                    ++ptr;
                }  
                ++ptr;
            }
            ++ptr;
        }



    }
    else if (iInput == 1)
    {

        JobForPIU_16S(fp, ptr, pEndPoint);

    }

    


    fprintf(fp, "-------------------------------------------------------------------------------------------------------------\n\n");
    fclose(fp);
    free(pFileContents);






}



int GotoStart(char** pptr,  const char* const pEndpoint)
{
    char* ptr = *pptr;
    int iFind = 0;

    char Buffer[sizeof(StartPoint)];


    while(ptr != pEndpoint)
    {
        
        if(*ptr == '0')
        {
            memset(Buffer, 0, sizeof(StartPoint));
            memcpy(Buffer, ptr, strlen(StartPoint));

            if(!strcmp(StartPoint, Buffer))
            {
                *pptr = ptr + strlen(StartPoint);
                iFind = 1;
                break;
            }
        }

        ++ptr;
    }


    return iFind;

}

int CheckMonth(char** pptr)
{
    char Buffer[df_MAX_BUFFER];

    for(int iCnt = 0; iCnt < df_MONTH ; ++iCnt)
    {
        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, *pptr, 3);
        if(!strcmp(Buffer, Mon[iCnt]))
        {
            *pptr += 3;
            return 1;
        }
    }

    return 0;
}








int StartPoint_PIU_16S(char** pptr, const char* const pEndPoint)
{
    char* ptr = *pptr;
    int iFind = 0;

    while(ptr != pEndPoint)
    {
        if(*ptr == 'D')
        {
            ++ptr;
            if(*ptr == ':')
            {
                *pptr = ++ptr;
                iFind = 1;
                break;
            }   
        }
        ++ptr;
    }
    return iFind;
}

char* GotoVal(char* ptr, const char dst)
{
    while(*ptr != dst)
        ++ptr;

    return ptr;
}

char* GotoEndOfString(char* ptr, const char* String)
{
    char StartString = *String;
    char TempBuf[df_MAX_BUFFER];


    while(1)
    {
        if(*ptr == StartString)
        {
            memset(TempBuf, 0, df_MAX_BUFFER);
            memcpy(TempBuf, ptr, strlen(String));
            if(!strcmp(TempBuf, String))
                return ptr + strlen(String);
        }
        ++ptr;
    }

    // 실행될 일 없음
    return NULL;
}





void JobForPIU_16S(FILE* fp, char* Paramptr, const char* const pEndPoint)
{
    char* pTemp;
    char* ptr = Paramptr;
    char Buffer[df_MAX_BUFFER];

   ContentsBuffer = (char*)malloc(df_MAX_CONTENTS_SIZE);

    while(ptr != pEndPoint)
    {
        if(!StartPoint_PIU_16S(&ptr, pEndPoint))
            break;

        // S: 전까지 ptr 이동
        pTemp = ++ptr;
        ptr = GotoVal(ptr, 'S');
        

        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, pTemp, (size_t)(ptr - pTemp));
        fprintf(fp, "%s", Buffer);


        // [ 전까지 ptr이동
        ptr += 2;
        pTemp = ptr;
        ptr = GotoVal(ptr, '[');

        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, pTemp, (size_t)(ptr - pTemp));
        fprintf(fp, "%s", Buffer);


        // ]전까지 ptr 이동
        ptr += 1;
        pTemp = ptr;
        ptr = GotoVal(ptr, ']');

        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, pTemp, (size_t)(ptr - pTemp));
        fprintf(fp, "%s\n", Buffer);

       
        // Subtypes 데이터 넣기
        ptr = GotoVal(ptr, ':');
        ptr += 2;
        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, ptr, 3);
        fprintf(fp, "%s", Buffer);

        // Flags 데이터 넣기
        ptr = GotoVal(ptr, ':');
        ++ptr;
        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, ptr, 6);
        fprintf(fp, "%s", Buffer);


        // Type 넣기
        ptr = GotoEndOfString(ptr, "Type");
        ++ptr;
        memset(Buffer, 0, df_MAX_BUFFER);
        memcpy(Buffer, ptr, 2);
        fprintf(fp, "%s ", Buffer);

        // OUI 넣기 끝까지!
        ptr = GotoEndOfString(ptr, "OUI");
        pTemp = ++ptr;

        while(*ptr != '[' && ptr != pEndPoint) 
        {
            if(ptr > pEndPoint)
            {
                printf("err!!!\n\n");
            }
            ++ptr;
        }


        ////////////////////////////////////////////////////////////
        //   주의!! : 현재 여기에서 10000바이트가 설정되어 있지만 이것보다 더 큰 값이 올 수 있다.
        ////////////////////////////////////////////////////////////
        memset(ContentsBuffer, 0, df_MAX_CONTENTS_SIZE);
        memcpy(ContentsBuffer, pTemp, (size_t)(ptr - pTemp));
        fprintf(fp, "%s\n\n", ContentsBuffer);
        
        if(ptr == pEndPoint)
        {
            printf("End!\n\n");
            break;
        }    

        
        fflush(fp);
    

        ++ptr;
    }


    free(ContentsBuffer);

}