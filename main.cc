#include <iostream>
#include <thread>
#include <cstring>
#include <string>
#include <ncurses.h>
#include <fstream>
#include <vector>
#include <random>
#include <unistd.h>
using std::cout;
using std::endl;

/*
 *  Trying to make a typing game to help you get better at typing.
 *  Saw the game bisqwit made and used in his 'how fast does bisqwit type' video.
 * */

#define DELAY 300000

class Word{
    public:
        std::string word;
        int x,y;
        Word(std::string w, int y){
            word = w;
            this->y = y;
            x = 0 - w.length();
        }
        void move(WINDOW *win){
            mvwprintw(win,y,x-1," ");
            mvwprintw(win,y,x,word.c_str());
            x++;
        }
};

class WordsWindow{
    private:
        std::random_device rd;/* Random number for random words from file */
        std::mt19937 rng;
        std::ifstream file;/* Open the file containing the words */
        /* Vector to hold the words and put the words into it */
        std::vector<std::string> words;
        std::string word;
        int r,c;
        std::vector<Word> current_words;
        WINDOW *top_win;
        int delay;
        int next_word_delay;
        bool game_over;

    public:
        WordsWindow(int row, int col):r(row),c(col){
            rng.seed(rd());
            std::string fname = "words.txt";
            file.open(fname);
            /* Check the file was opened OK */
            if(!file){
                cout << "Error opening file.\n";
                exit(1);
            }
            init();
        }
        void init(){
            while(std::getline(file, word)){
                words.push_back(word);
            }
            game_over=false;
            delay=0;
            next_word_delay=15;
            /* Init ncurses */
            // initscr();
            // clear();
            // getmaxyx(stdscr,row,col);		[> get the number of rows and columns <]
            top_win = newwin(r-1,c,0,0);
            // box(top_win,0,0);
            wrefresh(top_win);
            get_word(words,current_words,rng,r);
        }
        void get_word(std::vector<std::string> &words, std::vector<Word> &current_words, std::mt19937 &rng, int rows){
            int i = rng() % words.size();
            int y = (rng() % rows-2) +1;
            Word w(words[i],y);
            current_words.push_back(w);
            words.erase(words.begin() + i);
        }
        void clear_words(){wclear(top_win);}
        void update(){
            while(!game_over){
                for(size_t i=0; i<current_words.size(); i++){
                    if(current_words[i].x + current_words[i].word.length() > (unsigned int)c){
                        game_over = true;
                    }
                    current_words[i].move(top_win);
                }
                // box(top_win,0,0);
                wrefresh(top_win);

                // napms(500);
                usleep(DELAY);
                delay++;
                if(delay >= next_word_delay && !words.empty()){
                    delay=0;
                    get_word(words,current_words,rng,r);
                }
            }
        }
        std::thread play(){
            return std::thread(&WordsWindow::update, this);
        }
        bool check_match(std::string guess){
            for(size_t i=0; i<current_words.size(); i++){
                if(current_words[i].word == guess){
                    current_words.erase(current_words.begin() + i);
                    return true;
                }
            }
            return false;
        }
        ~WordsWindow(){
            /* Deallocate ncurses */
            delwin(top_win);
            /* Close the file */
            file.close();
        }
};

int main(void){

    /* Need to get height and width of terminal window, pick random height and random word,
     * start pushing random word from off screen left to off screen right.
     * Need to setup input location at bottom of screen.
     * As random words are chosen, maybe add them to a queue to be popped and pushed across screen.
     * Maybe instead of loading them into a vector, or as a random word is chosen it is removed fromt he wordlist
     * vector and pushed onto a current words vector, then when a word is typed in the input area, check if the word
     * is in the list of current words and if it is, remove it from the current words and add points to score.*/
    // int row,col;
    // getmaxyx(stdscr,row,col);		[> get the number of rows and columns <]

    // Figure out how I'm going to run the game loop
    // Figure out how I'm going to store the current words and setup function to grab random word from words and add it to the current words container and remove from words list container
    // Write function to pick random height to scroll word at.
    // Maybe make the word a data type that holds the word and the position
    // Maybe make a map to map the word in the current words to it's object that holds the word and position

    int row, col, score=0;
    /* Init ncurses */
    initscr();
    nocbreak();
    echo();
    clear();
    getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

    WordsWindow ww(row,col);
    ww.init();
    std::thread t1 = ww.play();

    bool playing=true;
    WINDOW *input_win = newwin(1,col,row-1,1);
    wrefresh(input_win);
    while(playing){
        std::string guess;
        std::string score_str="Score: ";
        score_str+=std::to_string(score);
        // mvwscanw(input_win,0,0,*guess);

        // OK this is working, todo: setup variable for score, probably set itt so that every loop score gets concatenated onto 'Score: '
        // Also setup way to check if guess string matches any of the current words
        // Maybe make game_over a static variable so that this loop can run based on that also
        //
        // That's all for now I guess, maybe later implement a timmer that increments a word counter everytime a word is removed from current_words
        // then update a variable based on real time, or maybe to start just have a 'game over' screen witth score and stats about words per minute

        int ch = mvwgetch(input_win,0,0);

        while(ch != '\n'){
            guess.push_back(ch);
            ch = mvwgetch(input_win,0,0);
        }
        if(ww.check_match(guess)){
            score++;
            score_str="Score: ";
            score_str+=std::to_string(score);
            ww.clear_words();
        }

        wclear(input_win);
        mvwprintw(input_win,0,col-11,score_str.c_str());
        mvwprintw(input_win,0,col/2,guess.c_str());

        wrefresh(input_win);

    }// End while game loop

    t1.join();

    /* Deallocate ncurses */
    delwin(input_win);
    endwin();

    return 0;
}