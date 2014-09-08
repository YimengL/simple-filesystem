/* Course: Operating Systems 416, Spring */
/* Assignment 7: math file system */
/* 
   Author: Zhenhua Jia
    	   Yimeng Li
    	   Yihan Qian	 
*/

# define FUSE_USE_VERSION 30

# include <fuse.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <fcntl.h>
# include <math.h>
# include <inttypes.h>

# define FILESIZE 800	/* define the maximum length of conten in buf */
# define Path_size 50	/* define the maximum length of a path */

/* directories */
static const char *add_str = "Add two numbers.\nThe file add/a/b contains the sum a+b.\n";
static const char *div_str = "Divide two numbers.\nThe file div/a/b contains the quotient a/b.\n";
static const char *sub_str = "Subtract two numbers.\nThe file sub/a/b contains the difference a-b.\n";
static const char *mul_str = "Multiply two numbers.\nThe file mul/a/b contains the product a*b.\n";
static const char *exp_str = "The file exp/a/b contains a raised to the power of b.\n";
static const char *fac_str = "The file fac/n contains the prime factors of n.\n";
static const char *fib_str = "The file fib/n contains the first n fibonacci numbers.\n";

static const char *add_path = "/add";
static const char *div_path = "/div";
static const char *sub_path = "/sub";
static const char *mul_path = "/mul";
static const char *exp_path = "/exp";
static const char *fac_path = "/fac";
static const char *fib_path = "/fib";

static const char *adddoc_path = "/add/doc";
static const char *divdoc_path = "/div/doc";
static const char *subdoc_path = "/sub/doc";
static const char *muldoc_path = "/mul/doc";
static const char *expdoc_path = "/exp/doc";
static const char *facdoc_path = "/fac/doc";
static const char *fibdoc_path = "/fib/doc";

char temp[Path_size];	/* for parse the command purpose */
char *arg[3];			/* store the parsed token */
char Path1[Path_size];	/* store the path of command with one parameter */
char Path2[Path_size];	/* store the path of command with two parameter */

/* dynamic array */
typedef struct 
{
	uint64_t *array;
	size_t used;	/* count the number of integers */
	size_t size;	/* count the size of this array */
} Array;

/* initialize the array */
void initArray(Array *a, size_t initialSize)
{
	a->array = (uint64_t *)malloc(initialSize * sizeof(uint64_t));
	a->used = 0;
	a->size = initialSize;
}

/* insert at the end */
int insertArray(Array *a, uint64_t element)
{
	if (a->used == a->size) 
	{
		a->size *= 2;
		a->array = (uint64_t *)realloc(a->array, a->size * sizeof(uint64_t));
	}	
	a->array[a->used++] = element;
	return a->used;
}

/*print the entire array */
void printArray(Array *a)
{
	int i = 0;
	for (; i < a->used; i++)
		printf("%" PRIu64"\n", a->array[i]);
}

/* free the array sapce */
void freeArray(Array *a)
{
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

/* check if a double is also an integer */
int IsInt(double d)
{
	int i = d;
	return ((d-i) == 0);
}

/* Fibonacci Number */
void Fibonacci (Array *a, double number)
{
	long n = round(number);
	
	printf("The fibonacci numbers are 1, 1, 2...\n");
	
	int i = 1;
	if (n >= 1)
		insertArray(a, i);
	if (n >= 2)
		insertArray(a, i++);
	
	for(; i < n; i++)
		insertArray(a, a->array[i-1] + a->array[i-2]);
}

/* A function to print all prime factors of a given number n */
void primeFactors(Array *a, double number)
{	
	long n = round(number);

	int i = 2;
	if (n%i == 0)
		insertArray(a, i);
		
    while (n%i == 0)
        n = n/i;
 
    /* n must be odd at this point.  
     * So we can skip one element (Note i = i +2) */
    for ( i = 3; i <= sqrt(n); i = i+2)
    {
		if (n%i == 0)
			insertArray(a, i);
		
        /* While i divides n, print i and divide n */
        while (n%i == 0)
            n = n/i;
    }
 
    /* In case where n is a prime number and greater than 2 */
    if (n > 2)
		insertArray(a, n);
}

/* Parse the path string into tokens */
int parseString(const char *path)
{
	int arc = 0;
	int i = 0;
	strcpy(temp, path);
	char *token;
	token = strtok(temp, "/\n");
	for(i = 0; token != NULL; ++i)
	{
		++arc;
		arg[i] = token;
		token = strtok(NULL, "/\n");
	}
	return arc;
}

/* get attributes function */
static int math_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	int arc = 0;

	arc = parseString(path);
	memset(stbuf, 0, sizeof(struct stat));
	
	/* define as a directory */
	if (strcmp(path, "/") == 0) 
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	
	/* define as a directory */
	else if (strcmp(path, add_path) == 0 || strcmp(path, sub_path) == 0
	|| strcmp(path, mul_path) == 0 || strcmp(path, div_path) == 0
	|| strcmp(path, fib_path) == 0 || strcmp(path, fac_path) == 0
	|| strcmp(path, exp_path) == 0) 
	{
		stbuf->st_mode = S_IFDIR| 0755;
		stbuf->st_nlink = 3;
	} 
	
	/* doc, define as a file */
	else if (strcmp(path, adddoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(add_str);
	}
	else if (strcmp(path, subdoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(div_str);
	}
	else if (strcmp(path, muldoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(sub_str);
	}
	else if (strcmp(path, divdoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(mul_str);
	}
	else if (strcmp(path, expdoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(exp_str);
	}
	else if (strcmp(path, fibdoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(fib_str);
	}
	else if (strcmp(path, facdoc_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(fac_str);
	}
	
	/* add, sub, mul, div, exp functions with one parameter, 
	 * define as a directory */
	else if((strncmp(path, add_path, 4) == 0 && (arc == 2))
	|| (strncmp(path, sub_path, 4) == 0 && (arc == 2))
	|| (strncmp(path, mul_path, 4) == 0 && (arc == 2))
	|| (strncmp(path, div_path, 4) == 0 && (arc == 2))
	|| (strncmp(path, exp_path, 4) == 0 && (arc == 2)))
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
		strcpy(Path1, path);
	}	
	
	/* add, sub, mul, div, exp functions with two parameters, 
	 * define as a file */
	else if((strncmp(path, add_path, 4) == 0 && (arc == 3))
	|| (strncmp(path, sub_path, 4) == 0 && (arc == 3))
	|| (strncmp(path, mul_path, 4) == 0 && (arc == 3))
	|| (strncmp(path, div_path, 4) == 0 && (arc == 3))
	|| (strncmp(path, exp_path, 4) == 0 && (arc == 3)))
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = Path_size;
		strcpy(Path2, path);
	}
	
	/* fac, fib functions with one parameter, define as a file */
	else if((strncmp(path, fib_path, 4) == 0 && (arc == 2))
	|| (strncmp(path, fac_path, 4) == 0 && (arc == 2)))
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;		
		stbuf->st_size = FILESIZE;
		strcpy(Path1, path);
	}

	else
		res = -ENOENT;

	return res;
}

static int math_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	printf("readdir(\"%s\")\n", path);

	(void) offset;
	(void) fi;

	if(strcmp(path, "/") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, add_path + 1, NULL, 0);
		filler(buf, sub_path + 1, NULL, 0);
		filler(buf, mul_path + 1, NULL, 0);
		filler(buf, div_path + 1, NULL, 0);
		filler(buf, fib_path + 1, NULL, 0);
		filler(buf, fac_path + 1, NULL, 0);
		filler(buf, exp_path + 1, NULL, 0);
	}
	
	if (strcmp(path, add_path) == 0 || strcmp(path, mul_path) == 0
	 || strcmp(path, sub_path) == 0 || strcmp(path, div_path) == 0
	 || strcmp(path, fib_path) == 0 || strcmp(path, fac_path) == 0
	 || strcmp(path, exp_path) == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "doc", NULL, 0);
	}
	
	if((strcmp(path, Path1) == 0) || (strcmp(path, Path2) == 0))
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
	}
	
	return 0;
}

static int math_open(const char *path, struct fuse_file_info *fi)
{
	printf("open(\"%s\"\n", path);

	//if(strcmp(path, adddoc_path) == 0 || strcmp(path, div_path) == 0 
	//|| strcmp(path, sub_path) == 0 || strcmp(path, mul_path) == 0)
	//{
		//if ((fi->flags & 3) != O_RDONLY)
			//return -EACCES;
	//}
	return 0;
}



static int math_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	printf("read(\"%s\"\n", path);

	size_t len;
	(void) fi;
	memset(buf, 0, sizeof(buf));
	
	/* build the documentation for each operation */
	if(strcmp(path, adddoc_path) == 0)
	{
		len = strlen(add_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, add_str + offset, size);
		} else
			size = 0;
	}		
	else if(strcmp(path, subdoc_path) == 0)
	{
		len = strlen(sub_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, sub_str + offset, size);
		} else
			size = 0;
	}
	else if(strcmp(path, muldoc_path) == 0)
	{
		len = strlen(mul_str);	
		if (offset < len) 
		{
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, mul_str + offset, size);
		} else
			size = 0;
	}
	else if(strcmp(path, divdoc_path) == 0)
	{
		len = strlen(div_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, div_str + offset, size);
		} else
			size = 0;
	}
	else if(strcmp(path, expdoc_path) == 0)
	{
		len = strlen(exp_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, exp_str + offset, size);
		} else
			size = 0;
	}
	
	else if(strcmp(path, facdoc_path) == 0)
	{
		len = strlen(fac_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, fac_str + offset, size);
		} else
			size = 0;
	}
	else if(strcmp(path, fibdoc_path) == 0)
	{
		len = strlen(fib_str);	
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, fib_str + offset, size);
		} else
			size = 0;
	}
	
	/* add, sub, mul, div, exp operations, 
	 * compute and dump the result into buf*/
	else if(strcmp(path, Path2) == 0)
	{
		double d[2];
		double result;
		char buffer[Path_size];
		
		d[0] = strtod (arg[1], NULL);
		d[1] = strtod (arg[2], NULL);
		
		if (strncmp(path, add_path, 4) == 0)
		{
			result = d[0] + d[1];
			sprintf(buffer, "%lf\n", result);
		}
		else if (strncmp(path, sub_path, 4) == 0)
		{
			result = d[0] - d[1];
			sprintf(buffer, "%lf\n", result);
		}
		else if (strncmp(path, mul_path, 4) == 0)
		{
			result = d[0] * d[1];
			sprintf(buffer, "%lf\n", result);
		}
		else if (strncmp(path, div_path, 4) == 0)
		{
			/* check if the divisor is 0 */
			if(d[1] != 0)
			{
				result = d[0] / d[1];
				sprintf(buffer, "%lf\n", result);
			}
			else
				strcpy(buffer, "divide by zero error\n");
		}
		else if (strncmp(path, exp_path, 4) == 0)
		{
			result = pow(d[0], d[1]);
			sprintf(buffer, "%lf\n", result);
		}

		/* get the length of the string, dump it into bug */
		len = strlen(buffer);		
		if (offset < len) 
		{
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, buffer + offset, size);
		} else
			size = 0;
	}
	
	/* fib, fac operations, 
	 * compute and dump the result into buf*/
	if(strcmp(path, Path1) == 0)
	{
		Array f;				
		double d;
		int i;
		char buffer[FILESIZE];		
		initArray(&f,1);		
		
		d = strtod (arg[1], NULL);
		memset(buffer, 0, sizeof(buffer));
		char tempbuf[100];
		
		if (strncmp(path, fib_path, 4) == 0)
		{
			/* check if input number is a positve integer */
			if (IsInt(d) && d >= 1)
			{
				Fibonacci(&f, d);
				printArray(&f);
				
				/* put fibonacci numbers into one buffer*/
				for (i=0; i < (&f)->used; i++)
				{
					sprintf(tempbuf, "%"PRIu64"\n", (&f)->array[i]) ;
					strcat(buffer, tempbuf);				
				}
				
			}
			else
				strcpy(buffer, "Only integers that are larger than 1 have prime factors.\n");
			
			/* get the length of the string, dump it into buf */	
			len = strlen(buffer);			
			if (offset < len)
			{
				if (offset + size > len)
					size = len - offset;
				memcpy(buf, buffer + offset, size);
			} 
		}
		else if (strncmp(path, fac_path, 4) == 0)
		{
			/* check if the input number is a integer > 2*/
			if (IsInt(d) && d > 2)
			{
				primeFactors(&f, d);
				for (i=0; i < (&f)->used; i++)
				{
					sprintf(tempbuf, "%"PRIu64"\n", (&f)->array[i]) ;
					strcat(buffer, tempbuf);				
				}
			}
			else 
				strcpy(buffer, "Error. Only positive integer is expected.\n");
			/* get the length of the string, dump it into buf */
			len = strlen(buffer);			
			if (offset < len) 
			{
				if (offset + size > len)
					size = len - offset;
				memcpy(buf, buffer + offset, size);
			} 
		}
		
		
		else
			size = 0;
		freeArray(&f);
	}

	return size;
}

static struct fuse_operations math_oper = {
	.getattr	= math_getattr,
	.readdir	= math_readdir,
	.open		= math_open,
	.read		= math_read,
};

int main(int argc, char *argv[])
{
	printf("argc: %d\n", argc);
	return fuse_main(argc, argv, &math_oper, NULL);
}
