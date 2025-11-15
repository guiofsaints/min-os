#define _GNU_SOURCE // for strcasestr
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include "defines.h"
#include "utils.h"

///////////////////////////////////////

int prefixMatch(const char* pre, const char* str) {
	return (strncasecmp(pre,str,strlen(pre))==0);
}
int suffixMatch(const char* suf, const char* str) {
	int len = strlen(suf);
	int offset = strlen(str)-len;
	return (offset>=0 && strncasecmp(suf, str+offset, len)==0);
}
int exactMatch(const char* str1, const char* str2) {
	if (!str1 || !str2) return 0; // NULL isn't safe here
	int len1 = strlen(str1);
	if (len1!=strlen(str2)) return 0;
	return (strncmp(str1,str2,len1)==0);
}
int containsString(const char* haystack, const char* needle) {
	return strcasestr(haystack, needle) != NULL;
}
int hide(const char* file_name) {
	return file_name[0]=='.' || suffixMatch(".disabled", file_name) || exactMatch("map.txt", file_name);
}
char *splitString(char *str, const char *delim)
{
    char *p = strstr(str, delim);
    if (p == NULL)
        return NULL;          // delimiter not found
    *p = '\0';                // terminate string after head
    return p + strlen(delim); // return tail substring
}
void truncateString(char *string, size_t max_len) {
	size_t len = strlen(string) + 1;
	if (len <= max_len) return;

	strncpy(&string[max_len - 4], "...\0", 4);
}
void wrapString(char *string, size_t max_len, size_t max_lines) {
	char *line = string;

	for (size_t i = 1; i < max_lines; i++) {
		char *p = line;
		char *prev;
		do {
			prev = p;
			p = strchr(prev+1, ' ');
		} while (p && p - line < (int)max_len);

		if (!p && strlen(line) < max_len) break;

		if (prev && prev != line) {
			line = prev + 1;
			*prev = '\n';
		}
	}
	truncateString(line, max_len);
}
// TODO: verify this yields the same result as the one in minui.c, remove one
// This one does not modify the input, cause we arent savages
char *replaceString2(const char *orig, const char *rep, const char *with)
{
    const char *ins;     // the next insert point
    char *tmp;     // varies
    int len_rep;   // length of rep (the string to remove)
    int len_with;  // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;     // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count)
        ins = tmp + len_rep;

    char *result =
        (char *)malloc(strlen(orig) + (len_with - len_rep) * count + 1);
    tmp = result;

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        size_t remaining = result_len - (tmp - result);
        strncpy(tmp, with, remaining - 1);
        tmp[remaining - 1] = '\0';
        tmp += len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strncpy(tmp, orig, result_len - (tmp - result) - 1);
    result[result_len - 1] = '\0';
    return result;
}
// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.
size_t trimString(char *out, size_t len, const char *str, bool first)
{
    if (len == 0)
        return 0;

    const char *end;
    size_t out_size;
    bool is_string = false;

    // Trim leading space
    while (strchr("\r\n\t {},", (unsigned char)*str) != NULL)
        str++;

    end = str + 1;

    if ((unsigned char)*str == '"') {
        is_string = true;
        str++;
        while (strchr("\r\n\"", (unsigned char)*end) == NULL)
            end++;
    }

    if (*str == 0) // All spaces?
    {
        *out = 0;
        return 1;
    }

    // Trim trailing space
    if (first)
        while (strchr("\r\n\t {},", (unsigned char)*end) == NULL)
            end++;
    else {
        end = str + strlen(str) - 1;
        while (end > str && strchr("\r\n\t {},", (unsigned char)*end) != NULL)
            end--;
        end++;
    }

    if (is_string && (unsigned char)*(end - 1) == '"')
        end--;

    // Set output size to minimum of trimmed string length and buffer size minus
    // 1
    out_size = (end - str) < len - 1 ? (end - str) : len - 1;

    // Copy trimmed string and add null terminator
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

void removeParentheses(char *str_out, const char *str_in)
{
    char temp[STR_MAX];
    int len = strlen(str_in);
    int c = 0;
    bool inside = false;
    char end_char;

    for (int i = 0; i < len && i < STR_MAX; i++) {
        if (!inside && (str_in[i] == '(' || str_in[i] == '[')) {
            end_char = str_in[i] == '(' ? ')' : ']';
            inside = true;
            continue;
        }
        else if (inside) {
            if (str_in[i] == end_char)
                inside = false;
            continue;
        }
        temp[c++] = str_in[i];
    }

    temp[c] = '\0';

    trimString(str_out, STR_MAX - 1, temp, false);
}
void serializeTime(char *dest_str, int nTime)
{
    if (nTime >= 60) {
        int h = nTime / 3600;
        int m = (nTime - 3600 * h) / 60;
        if (h > 0) {
            snprintf(dest_str, 256, "%dh %dm", h, m);
        }
        else {
            snprintf(dest_str, 256, "%dm %ds", m, nTime - 60 * m);
        }
    }
    else {
        snprintf(dest_str, 256, "%ds", nTime);
    }
}
int countChar(const char *str, char ch)
{
    int i, count = 0;
    for (i = 0; i <= strlen(str); i++) {
        if (str[i] == ch) {
            count++;
        }
    }
    return count;
}
char *removeExtension(const char *myStr)
{
    if (myStr == NULL)
        return NULL;
    char *retStr = (char *)malloc(strlen(myStr) + 1);
    char *lastExt;
    if (retStr == NULL)
        return NULL;
    strncpy(retStr, myStr, strlen(myStr));
    retStr[strlen(myStr)] = '\0';
    if ((lastExt = strrchr(retStr, '.')) != NULL && *(lastExt + 1) != ' ' && *(lastExt + 2) != '\0')
        *lastExt = '\0';
    return retStr;
}
const char *baseName(const char *filename)
{
    char *p = strrchr(filename, '/');
    return p ? p + 1 : (char *)filename;
}
void folderPath(const char *path, char *result) {
    char pathCopy[256];  
    strncpy(pathCopy, path, sizeof(pathCopy) - 1);
    pathCopy[sizeof(pathCopy) - 1] = '\0';

    char *lastSlash = strrchr(pathCopy, '/');  // Find the last slash
    if (lastSlash != NULL) {
        *lastSlash = '\0';  // Cut off the filename
        strncpy(result, pathCopy, 256 - 1);
        result[255] = '\0';  // Copy the remaining path
    } else {
        result[0] = '\0';  // No folder found
    }
}
void cleanName(char *name_out, const char *file_name)
{
    char *name_without_ext = removeExtension(file_name);
    char *no_underscores = replaceString2(name_without_ext, "_", " ");
    char *dot_ptr = strstr(no_underscores, ".");
    if (dot_ptr != NULL) {
        char *s = no_underscores;
        while (isdigit(*s) && s < dot_ptr)
            s++;
        if (s != dot_ptr)
            dot_ptr = no_underscores;
        else {
            dot_ptr++;
            if (dot_ptr[0] == ' ')
                dot_ptr++;
        }
    }
    else {
        dot_ptr = no_underscores;
    }
    removeParentheses(name_out, dot_ptr);
    free(name_without_ext);
    free(no_underscores);
}
bool pathRelativeTo(char *path_out, const char *dir_from, const char *file_to)
{
    path_out[0] = '\0';

    char abs_from[MAX_PATH];
    char abs_to[MAX_PATH];
    if (realpath(dir_from, abs_from) == NULL || realpath(file_to, abs_to) == NULL) {
        return false;
    }

    char *p1 = abs_from;
    char *p2 = abs_to;
    while (*p1 && (*p1 == *p2)) {
        ++p1, ++p2;
    }

    if (*p2 == '/') {
        ++p2;
    }

    if (strlen(p1) > 0) {
        int num_parens = countChar(p1, '/') + 1;
        for (int i = 0; i < num_parens; i++) {
            strncat(path_out, "../", MAX_PATH - strlen(path_out) - 1);
        }
    }
    strncat(path_out, p2, MAX_PATH - strlen(path_out) - 1);

    return true;
}

void getDisplayName(const char* in_name, char* out_name) {
	char* tmp;
	char work_name[256];
	strncpy(work_name, in_name, sizeof(work_name) - 1);
	work_name[sizeof(work_name) - 1] = '\0';
	strncpy(out_name, in_name, 256 - 1);
	out_name[255] = '\0';
	
	if (suffixMatch("/" PLATFORM, work_name)) { // hide platform from Tools path...
		tmp = strrchr(work_name, '/');
		tmp[0] = '\0';
	}
	
	// extract just the filename if necessary
	tmp = strrchr(work_name, '/');
	if (tmp) {
		strncpy(out_name, tmp + 1, 256 - 1);
		out_name[255] = '\0';
	}
	
	// remove extension(s), eg. .p8.png
	while ((tmp = strrchr(out_name, '.'))!=NULL) {
		int len = strlen(tmp);
		if (len>2 && len<=5) tmp[0] = '\0'; // 1-4 letter extension plus dot (was 1-3, extended for .doom files)
		else break;
	}
	
	// remove trailing parens (round and square)
	strncpy(work_name, out_name, sizeof(work_name) - 1);
	work_name[sizeof(work_name) - 1] = '\0';
	while ((tmp=strrchr(out_name, '('))!=NULL || (tmp=strrchr(out_name, '['))!=NULL) {
		if (tmp==out_name) break;
		tmp[0] = '\0';
		tmp = out_name;
	}
	
	// make sure we haven't nuked the entire name
	if (out_name[0]=='\0') {
		strncpy(out_name, work_name, 256 - 1);
		out_name[255] = '\0';
	}
	
	// remove trailing whitespace
	tmp = out_name + strlen(out_name) - 1;
    while(tmp>out_name && isspace((unsigned char)*tmp)) tmp--;
    tmp[1] = '\0';
}
void getEmuName(const char* in_name, char* out_name) { // NOTE: both char arrays need to be MAX_PATH length!
	char* tmp;
	strncpy(out_name, in_name, MAX_PATH - 1);
	out_name[MAX_PATH - 1] = '\0';
	tmp = out_name;
	
	// printf("--------\n  in_name: %s\n",in_name); fflush(stdout);
	
	// extract just the Roms folder name if necessary
	if (prefixMatch(ROMS_PATH, tmp)) {
		tmp += strlen(ROMS_PATH) + 1;
		char* tmp2 = strchr(tmp, '/');
		if (tmp2) tmp2[0] = '\0';
		// printf("    tmp1: %s\n", tmp);
		strncpy(out_name, tmp, MAX_PATH - 1);
		out_name[MAX_PATH - 1] = '\0';
		tmp = out_name;
	}

	// finally extract pak name from parenths if present
	tmp = strrchr(tmp, '(');
	if (tmp) {
		tmp += 1;
		// printf("    tmp2: %s\n", tmp);
		strncpy(out_name, tmp, MAX_PATH - 1);
		out_name[MAX_PATH - 1] = '\0';
		tmp = strchr(out_name,')');
		tmp[0] = '\0';
	}
	
	// printf(" out_name: %s\n", out_name); fflush(stdout);
}
void getEmuPath(const char* emu_name, char* pak_path) {
	snprintf(pak_path, 256, "%s/Emus/%s/%s.pak/launch.sh", SDCARD_PATH, PLATFORM, emu_name);
	if (exists(pak_path)) return;
	snprintf(pak_path, 256, "%s/Emus/%s.pak/launch.sh", PAKS_PATH, emu_name);
}

void normalizeNewline(char* line) {
	int len = strlen(line);
	if (len>1 && line[len-1]=='\n' && line[len-2]=='\r') { // windows!
		line[len-2] = '\n';
		line[len-1] = '\0';
	}
}
void trimTrailingNewlines(char* line) {
	int len = strlen(line);
	while (len>0 && line[len-1]=='\n') {
		line[len-1] = '\0'; // trim newline
		len -= 1;
	}
}
void trimSortingMeta(char** str) { // eg. `001) `
	// TODO: this code is suss
	char* safe = *str;
	while(isdigit(**str)) *str += 1; // ignore leading numbers

	if (*str[0]==')') { // then match a closing parenthesis
		*str += 1;
	}
	else { //  or bail, restoring the string to its original value
		*str = safe;
		return;
	}
	
	while(isblank(**str)) *str += 1; // ignore leading space
}

///////////////////////////////////////

int exists(const char* path) {
	return access(path, F_OK)==0;
}
void touch(const char* path) {
	close(open(path, O_RDWR|O_CREAT, 0777));
}
int toggle(const char *path) {
    if (access(path, F_OK) == 0) {
        unlink(path);
        return 0;
    } else {
        touch(path);
        return 1;
    }
}
void putFile(const char* path, const char* contents) {
	FILE* file = fopen(path, "w");
	if (file) {
		fputs(contents, file);
		fclose(file);
	}
}
void getFile(const char* path, char* buffer, size_t buffer_size) {
	FILE *file = fopen(path, "r");
	if (file) {
		fseek(file, 0L, SEEK_END);
		size_t size = ftell(file);
		if (size>buffer_size-1) size = buffer_size - 1;
		rewind(file);
		fread(buffer, sizeof(char), size, file);
		fclose(file);
		buffer[size] = '\0';
	}
}
char* allocFile(const char* path) { // caller must free!
	char* contents = NULL;
	FILE *file = fopen(path, "r");
	if (file) {
		fseek(file, 0L, SEEK_END);
		size_t size = ftell(file);
		contents = calloc(size+1, sizeof(char));
		fseek(file, 0L, SEEK_SET);
		fread(contents, sizeof(char), size, file);
		fclose(file);
		contents[size] = '\0';
	}
	return contents;
}
int getInt(const char* path) {
	int i = 0;
    if(path == NULL)
        return i;
    
	FILE *file = fopen(path, "r");
	if (file!=NULL) {
		int res = fscanf(file, "%i", &i);
		fclose(file);
        if(res != 1)
            i = 0; // failed to parse int
	}
	return i;
}
void putInt(const char* path, int value) {
	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", value);
	putFile(path, buffer);
}

uint64_t getMicroseconds(void) {
    uint64_t ret;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    ret = (uint64_t)tv.tv_sec * 1000000;
    ret += (uint64_t)tv.tv_usec;

    return ret;
}

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

int clamp(int x, int lower, int upper)
{
    return min(upper, max(x, lower));
}

double clampd(double x, double lower, double upper)
{
    return min(upper, max(x, lower));
}