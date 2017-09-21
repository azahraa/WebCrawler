
#include "SimpleHTMLParser.h"
#include "openhttp.h"
#include <string.h>

char *m = new char[500];
int x = 0;
int cnt = 0;

SimpleHTMLParser::SimpleHTMLParser()
{
}

bool
SimpleHTMLParser::Contains(char *str, const char *sub) {
	for(int i = 0; i < strlen(str); i++)
	{
		if(str[i] == sub[0])
		{
			int j = 0;
			int k = i;
			int count = 0;
			while(j < strlen(sub) && k < strlen(str))
			{
				if(sub[j] == str[k])
				{
					count++;
				}
				j++;
				k++;
			}
			if(count == strlen(sub))
			{
				return true;
			}
		}
	}
	return false;
}

bool
SimpleHTMLParser::match(char **b, const char *m) {
	int len = strlen(m);
	if ( !strncasecmp(*b, m, len)) {
		(*b)+=len;
		return true;
	}
	return false;
}

bool
SimpleHTMLParser::parse(char * buffer, int n)
{
	enum { START, TAG, SCRIPT, ANCHOR, HREF,
	       COMMENT, FRAME, STYLE, TITLE, META, METAKEY, SRC } state;

	state = START;
	char * bufferEnd = buffer + n;
	char * b = buffer;
	bool lastCharSpace = false;
	bool validHTML = false;
	if(match(&b,"<!DOCTYPE html>"))
	{
		validHTML = true;
	}

	while (validHTML == true && b < bufferEnd) {
		//printf("<%c,%d,%d>", *b, *b,state);
		switch (state) {
		case START: {
			if (match(&b,"<title>")) {
				state = TITLE;
			}
			else if (match(&b,"<SCRIPT")) {
				state = SCRIPT;
			}
			else if (match(&b,"<!--")) {
				state = COMMENT;
			}
			else if (match(&b,"<A ")) {
				state = ANCHOR;
			}
			else if (match(&b,"<FRAME ")) {
				state = FRAME;
			}
			else if (match(&b,"<META NAME=\"keywords\" content=\"")) {
				state = METAKEY;
			}
			else if (match(&b,"<META NAME=\"description\" content=\"")) {
				state = METAKEY;
			}
			else if (match(&b,"<META CONTENT=\"")) {
				memset(m,0,500*sizeof(char));
				cnt = 0;
				state = META;
			}
			else if	(match(&b,"<STYLE ") || match(&b,"<style ")) {
				state = STYLE;
			}
			else if	(match(&b,"<")) {
				state = TAG;
			}
			else {
				char c = *b;
				//Substitute one or more blank chars with a single space
				if (c=='\n'||c=='\r'||c=='\t'||c==' ') {
					if (!lastCharSpace) {
						onContentFound(' ');
					}
					lastCharSpace = true;
				}
				else {
					onContentFound(c);
					lastCharSpace = false;
				}

				b++;
			}
			break;
		}
		case ANCHOR: {
			if (match(&b,"href=\"")) {
				state = HREF;
				urlAnchorLength=0;
				//printf("href=");
			}
			else if (match(&b,">")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;

		}
		case HREF: {
			if (match(&b,"\"")) {
				// Found ending "
				state = ANCHOR;
				urlAnchor[urlAnchorLength] = '\0';
				int skip = 0;
				int g = urlAnchorLength;

				if(urlAnchor[0] == '#' || urlAnchor[0] == '.' || urlAnchor[0] == 'f')
				{
					skip = 1;
				}

				if(Contains(urlAnchor, "docs") || Contains(urlAnchor, ".pdf") || Contains(urlAnchor, ".PDF") || Contains(urlAnchor, "article"))
				{
					skip = 1;
				}
				if(!skip)
				{
					onAnchorFound(urlAnchor);
				}
				//printf("\n");
			}
			else {
				if ( urlAnchorLength < MaxURLLength-1) {
					urlAnchor[urlAnchorLength] = *b;
					urlAnchorLength++;
				}
				//printf("%c", *b, *b);
				b++;
			}
			break;
		}
		case FRAME: {
			if (match(&b,"src=\"")) {
				state = SRC;
				urlAnchorLength=0;
				//printf("href=");
			}
			else if (match(&b,">")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;

		}
		case SRC: {
			if (match(&b,"\"")) {
				// Found ending "
				state = FRAME;
				urlAnchor[urlAnchorLength] = '\0';
				onAnchorFound(urlAnchor);
				//printf("\n");
			}
			else {
				if ( urlAnchorLength < MaxURLLength-1) {
					urlAnchor[urlAnchorLength] = *b;
					urlAnchorLength++;
				}
				//printf("%c", *b, *b);
				b++;
			}
			break;
		}
		case SCRIPT: {
			if (match(&b,"/SCRIPT>")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case COMMENT: {
			if (match(&b,"-->")) {
				// End comments
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case STYLE: {
			if (match(&b,"/STYLE>") || match(&b,"/style>")) {
				// End style
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case META: {
			if (match(&b, "\" name=\"description\"")||match(&b, "\" name=\"keywords\"")) {
				//if(l!=0)
				//	onContentFound(' ');
				for(int i = 0; i < 500; i++) {
					if(m[i]==0)
						break;
					else
						onContentFound(m[i]);

				}
				onContentFound('.');
				onContentFound(' ');
				state = START;
			}
			else if (match(&b,">")) {
				state = START;
			}
			else if (match(&b,"$description.value\"")) {
				state = TAG;
			}
			else if (match(&b,"$keywords.value\"")) {
				state = TAG;
			}
			else {
				char d = *b;
				//Substitute one or more blank chars with a single space
				//if (d=='\n'||d=='\r'||d=='\t'||d==' ') {
				if (!(('a'<= d && d <= 'z') || ('A' <= d && d <= 'Z') || ('0' <= d && d <= '9') || (d == '-')
			 || (d == '.') || (d == ',') || (d == '\''))) {
					if (!lastCharSpace && cnt < 500) {
						m[cnt++] = ' ';
					}
					lastCharSpace = true;
				}
				else if(cnt < 500) {
					m[cnt++] = *b;
					lastCharSpace = false;
				}
				b++;
			}
			break;
		}
		case METAKEY: {
			if (match(&b,"\" />") || match(&b,"\"/>") || match(&b,"/>") || match(&b,"\" >") || match(&b,"\">") || match(&b,">")) {
				state = START;
			}

			else {
				char c = *b;
				//Substitute one or more blank chars with a single space
				//if (c=='\n'||c=='\r'||c=='\t'||c==' ') {
				if (!(('a'<= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '0') || (c == '-')
			 || (c == '.') || (c == ','))) {
					if (!lastCharSpace) {
						onContentFound(' ');
					}
					lastCharSpace = true;
				}
				else {
					onContentFound(c);
					lastCharSpace = false;
				}

				b++;
			}
			break;
		}
		case TITLE : {
			if (match(&b, "</TITLE>")){
				onContentFound('~');
				state = START;
				//onContentFound('*');
			}
			else {
				char c = *b;
				//Substitute one or more blank chars with a single space
				//if (c=='\n'||c=='\r'||c=='\t'||c==' ') {
				if (!(('a'<= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '-')
			 || (c == '.') || (c == ','))) {
					if (!lastCharSpace) {
						onContentFound(' ');
					}
					lastCharSpace = true;
				}
				else {
					onContentFound(c);
					lastCharSpace = false;
				}

				b++;
			}
			break;
		}
		case TAG: {
			if (match(&b, ">")) {
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		default:;
		}

	}
}

void
SimpleHTMLParser::onContentFound(char c) {
}

void
SimpleHTMLParser::onAnchorFound(char * url) {
}
