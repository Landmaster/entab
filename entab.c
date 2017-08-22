// Program to entab files.

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <sys/stat.h>

const char *argp_program_version = "entab 0.0";
static char doc[] = "Entab a file.";
static char args_doc[] = "[FILE]";
static struct argp_option options[] = { 
	{"spaces", 'n', "NSPACES", 0, "Number of spaces"},
	{"output", 'o', "FILE", 0, "File to output to instead of stdout"},
	{ 0 } 
};

struct arguments {
	size_t nspaces;
	const char *infile;
	const char *outfile;
};

static bool is_same_file(const char* sA, const char* sB) {
	struct stat A,B;
	stat(sA, &A);
	stat(sB, &B);

	return A.st_dev == B.st_dev && A.st_ino == B.st_ino;
}

static void external_entab(FILE *infile, FILE *outfile, int nspaces) {
	char buf[1024];
	const char *p;
	bool replace = true;
	size_t i = 0;
	while (fgets(buf,sizeof buf,infile)) {
		p = buf;
		while (replace) {
			if (*p == ' ') { // space?
				if (++i >= nspaces) { // keep track of spaces, and replace when the threshold is reached.
					fputs("\t",outfile);
					i = 0;
				}
				++p;
			} else if (*p == '\t') { // ignore already-present tabs
				fputs("\t",outfile);
				i = 0;
				++p;
			} else { // disable entabbing after all of the whitespace characters have been handled
				replace = false;
			}
		}
		fputs(p,outfile);
		if (buf[strlen(buf)-1] == '\n') {
			replace = true;
			i = 0;
		}
	}
}

static void internal_entab(FILE *file, int nspaces) {
	FILE *temp = tmpfile();
	external_entab(file, temp, nspaces);
	char buf[1024];
	while (fgets(buf, sizeof buf,temp)) {
		fputs(buf, file);
	}
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch (key) {
		case 'n':
			sscanf(arg,"%zd",&arguments->nspaces);
			break;
		case 'o':
			arguments->outfile = arg;
			break;
		case ARGP_KEY_ARG:
			if (!arguments->infile)
				arguments->infile = arg;
			else
				argp_usage(state);
			break;
		case ARGP_KEY_END:
			if (!arguments->nspaces) argp_usage(state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}   
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char *argv[]) {
	struct arguments arguments = {0, 0, 0};
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	
	FILE *infile = stdin, *outfile = stdout;
	
	bool same = arguments.infile && arguments.outfile && is_same_file(arguments.infile, arguments.outfile);
	
	if (arguments.infile) {
		infile = fopen(arguments.infile,"rb+");
	}
	if (!infile) {
		fprintf(stderr,"Error: file %s could not be opened\n",arguments.infile);
		return 1;
	}
	
	if (same) {
		internal_entab(infile, arguments.nspaces);
	} else {
		if (arguments.outfile) {
			outfile = fopen(arguments.outfile,"wb");
		}
		if (!outfile) {
			fprintf(stderr,"Error: file %s could not be created\n",arguments.outfile);
			return 1;
		}
		
		external_entab(infile, outfile, arguments.nspaces);
	}
	
	
	int status = !feof(infile);
	if (infile != stdin) fclose(infile);
	if (outfile != stdout) fclose(outfile);
	return status;
}