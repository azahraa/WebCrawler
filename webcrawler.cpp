
#include "webcrawler.h"
#include "SimpleHTMLParser.h"

// Add your implementation here

char *description = new char[501]; // One for '\0'
int count_description = 0;
char *word = new char[16];
int count_word = 0;
int description_ctrl = 1;

WebCrawler::WebCrawler(int maxUrls, int nInitialUrls, const char **initialUrls)
{
  _maxUrls = maxUrls;
  _headURL = 0;
  _tailURL = nInitialUrls;
  _urlArray = new URLRecord[maxUrls];
  _urlToUrlRecord = new HashTableTemplate<int>();
  _wordToURLRecordList = new HashTableTemplate<URLRecordList*>();

  for(int i = 0; i < nInitialUrls; i++)
  {
    char *temp = strdup(initialUrls[i]);
    _urlArray[i]._url = temp;
    _urlArray[i]._description = NULL;
    _urlToUrlRecord -> insertItem(temp,i);
  }

}

char*
WebCrawler::ValidateURL(char *url)
{
  if(strlen(url) > 2)
  {
    char *root1 = _urlArray[_headURL]._url;
    char last = root1[strlen(root1) - 1];
    char *root = root1;
    if(last != '/')
    {
      char *tmp0 = new char[strlen(root) + 2];
      tmp0 = strcpy(tmp0, root);
      tmp0 = strcat(tmp0, "/");
      root = tmp0;
    }

    if(url[0] == '/' && url[1] == '/')
      {
        char *tmp2 = new char[strlen(url) + 7];
        tmp2 = strcpy(tmp2, "https:");
        tmp2 = strcat(tmp2, url);
        url = tmp2;
      }
      else if(url[0] == '/')
      {
        char *tmp5 = new char[strlen(url) + strlen(root1) + 1];
        tmp5 = strcpy(tmp5, root1);
        tmp5 = strcat(tmp5, url);
        url = tmp5;
      }
      else if(!(url[0] == 'h' && url[1] == 't'))
      {
        char *tmp4 = new char[strlen(root) + strlen(url) + 1];
        tmp4 = strcpy(tmp4, root);
        tmp4 = strcat(tmp4, url);
        url = tmp4;
      }
  }
    return url;
}

void
WebCrawler::onAnchorFound(char *url)
{
  if(_tailURL < _maxUrls)
  {
    char *fixedURL = ValidateURL(url);
    int temp;
    bool isFound = _urlToUrlRecord -> find(fixedURL, &temp);
    if(isFound == false && fixedURL != NULL)
    {
      char *new_url = strdup(fixedURL);
    //  delete [] fixedURL;
      _urlArray[_tailURL]._url = new_url;
      _urlArray[_tailURL]._description = NULL;
      _urlToUrlRecord -> insertItem(new_url, _tailURL);
      _tailURL++;
    }
  }
}

bool
WebCrawler::ValidateC(char c)
{
  return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == ' ') || (c == '-')
 || (c == '.') || (c == ',') || (c == '\''));
}

void
WebCrawler::onContentFound(char c)
{
  if(c == '~')
  {
    description_ctrl = 0;
  }
  if(ValidateC(c))
  {
    if(description_ctrl && count_description < 500)
    {
      description[count_description++] = c;
    }
    if(c >= 'A' && c <= 'Z')
    {
      c = c + 32;
    }
    if(c != '\t' && c != '\n' && c != '\0' && c != ' ' && c != '-' && c != '.' && c != ',' && c != '\'')
    {
      if(count_word < 15)
      {
        word[count_word++] = c;
      }
    }
    else
    {
      word[count_word] = '\0';
      URLRecordList *x = NULL;
      if(word != NULL)
      {
        char *nWord = strdup(word);
        if(_wordToURLRecordList -> find(word, &x) == false)
        {
          URLRecordList *rec = new URLRecordList();
          rec -> _urlRecordIndex = _headURL;
          rec -> _next = NULL;
          _wordToURLRecordList -> insertItem(nWord, rec);
        }
        else
        {
          int found = 0;
          URLRecordList *tempRec = x;
          while(tempRec != NULL)
          {
            if(tempRec -> _urlRecordIndex == _headURL)
            {
              found = 1;
              break;
            }
            tempRec = tempRec -> _next;
          }
          if(!found)
          {
            URLRecordList *r = new URLRecordList();
            r -> _urlRecordIndex = _headURL;
      			r -> _next = x;
  					_wordToURLRecordList -> insertItem(nWord, r);
          }
        }
      }

      for(int h = 0;h < count_word; h++)
      {
        word[h] = 0;
      }
      count_word = 0;
    }
  }
}

void
WebCrawler::crawl()
{
  int x = 0;
  int temp;

  while(_headURL < _tailURL)
  {
    char *curr = _urlArray[_headURL]._url;//    if(curr != NULL)
    char *buffer = fetchHTML(curr, &temp);
    if(buffer != NULL)
    {
      description_ctrl = 1;
      parse(buffer, temp);
    }

    // Copy description into _description

      description[count_description++] = '\0';
      _urlArray[_headURL]._description = strdup(description);

      for(int j = 0; j < count_description; j++)
      {
        description[j] = 0;
      }
      count_description = 0;

    _headURL++;
  }
}


// Write array of URLs and descriptions to file
void
WebCrawler::writeURLFile(const char * urlFileName)
{
  remove(urlFileName);
  FILE *file;
  file = fopen(urlFileName, "w");
  for(int i = 0; i < _tailURL; i++)
  {
    fprintf(file, "%d %s\n", i + 1, _urlArray[i]._url);
    if (_urlArray[i]._description[0] != '\0')//_urlArray[i]._description != NULL)
    {
      fprintf(file, "%s\n", _urlArray[i]._description);
    }
    else
    {
      fprintf(file, "%s\n", "No description was found.");
    }
      fprintf(file, "\n");
  }

  fclose(file);
}

// Write list of words with their urls to file
void
WebCrawler::writeWordFile(const char *wordFileName)
{
  remove(wordFileName);
	FILE *file2 = fopen(wordFileName, "a");
  HashTableTemplateIterator<URLRecordList*> iterator(_wordToURLRecordList);
  const char *key;
  URLRecordList *p;
  while (iterator.next(key, p))
   {
     fprintf(file2, "%s ",key);
     while(p!= NULL)
     {
       fprintf(file2, "%d ", p-> _urlRecordIndex);
  	   p = p -> _next;
  	 }
  	 fprintf(file2,"\n");
   }
}




int main(int argc, char **argv)
{

  if (argc < 2)
    {
        fprintf( stderr, " Usage: webcrawl [-u <maxurls>] url-list\n");
    }
    else
    {
      int maxURLs = 1000;
      int count = 0;
      const char **initialURLs;
      int x = 1;

      if(!strcmp(argv[1], "-u"))
      {
        x = 3;
        maxURLs = atoi(argv[2]);
      }

      initialURLs = new const char*[argc - x];
      for (int i = 0 ; i < argc - x; i++)
      {
        initialURLs[i] = argv[i+x];
        count++;
      }

      WebCrawler *w = new WebCrawler(maxURLs,count,initialURLs);
      w -> crawl();
  		w -> writeURLFile("url.txt");
  		w -> writeWordFile("word.txt");
    }

    return 0;
}
