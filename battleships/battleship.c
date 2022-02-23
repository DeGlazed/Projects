#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<unistd.h>
#include<time.h>
#include<string.h>

int ok(int m[10][10], int dir, int pozi, int pozj, int dim);
void place(int m[10][10], int dir, int pozi, int pozj, int dim);
void pcmat(int m[10][10]);
void playermat(int m[10][10], char *file);
int destory_in_advance(WINDOW *w,int player_mat[10][10], int pc_mat[10][10], int *player_score, int *pc_score);

void newgame(WINDOW *w, char *file, int cp_player_mat[10][10], int cp_pc_mat[10][10], int *cp_player_score,int *cp_pc_score,int *charges,int *enable)
{
    FILE* fin=fopen(file,"r");

    int i,j,key,ci,cj;
    int player_score = 0;
    int pc_score = 0;
    int pc_mat[10][10];
    int player_mat[10][10];
    int turn=1; // -1 pt randul pc-ului ; 1 pt randul player-ului
    int geni,genj;
    int ok;
    int d;

    *charges=2; // salve pentru destroy in advance

    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);

    srand(time(NULL));
    initscr();

    w=newwin(100,100,1,1);

    wrefresh(w);

    pcmat(pc_mat);
    playermat(player_mat,file);

    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
            mvwprintw(w,i,j,"%d",player_mat[i][j]);

    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
            mvwprintw(w,i,j+12,"%c",254);

    // +12 pt a parcurge vizual pc_mat

    i=0;
    j=0;
    mvwprintw(w,i,j+12,"+");
    keypad(w,TRUE);//enable keyboard input
    
    mvwprintw(w,11,0,"Player's turn                                       ");
    mvwprintw(w,12,0,"P1 Score :%d",player_score);
    mvwprintw(w,12,12,"PC Score :%d",pc_score);

    mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);

    while((key=wgetch(w))!='q' && player_score < 20 && pc_score < 20)
    {
        if(*charges>0)
        {
            mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);
        }
        else
        {
            mvwprintw(w,15,0,"Nu mai ai salve pentru d");    
        }
        wrefresh(w);

        if(turn == 1) //randul jucatorului
        {
        
            mvwprintw(w,11,0,"Player's turn                                       ");
            mvwprintw(w,12,0,"P1 Score :%d",player_score);
            mvwprintw(w,12,12,"PC Score :%d",pc_score);
            wrefresh(w);
            ci=i;
            cj=j;

            if (key==10)
            {
                if(pc_mat[i][j]==1)
                {
                    player_score++;
                    mvwprintw(w,11,0,"lovit                                   ");
                    mvwprintw(w,12,0,"P1 Score :%d",player_score);
                    wattron(w, COLOR_PAIR(1));
                    mvwprintw(w,i,j+12,"X");
                    wattroff(w, COLOR_PAIR(1));

                    pc_mat[i][j]=3; // 3 == casuta aleasa si barca lovita

                    wrefresh(w);
                }
                else
                    if(pc_mat[i][j]==3 || pc_mat[i][j]==4)
                    {
                        mvwprintw(w,11,0,"deja lovit                          ");
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);
                    }
                    else
                    {
                        pc_mat[i][j]=4; // 4 == casuta aleasa si barca ratata

                        mvwprintw(w,11,0,"ratat, press any key for the pc turn            ");
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);
                        mvwprintw(w,i,j,"X");
                        turn = -1;
                    }
                
            }
//problema intalnita : destroy in advance da crash aplicatiei daca spamez prea mult
//am permis folosiarea a doua salve de tunuri prntru destroy in advance

            else
            {
                if(key=='d' && *charges>0)
                {
                    d=destory_in_advance(w,player_mat,pc_mat,&player_score,&pc_score);
                    if(d==0)
                    {
                        mvwprintw(w,13,0,"nu se poate folosi functia destroy in advance, press any key to continue");
                        wrefresh(w);
                        mvwprintw(w,13,0,"                                                                        ");
                        wrefresh(w);
                        wgetch(w);
                    }
                    else
                    {
                        *charges=*charges-1;
                        for(i=0;i<10;i++)
                            for(j=0;j<10;j++)
                            {
                                mvwprintw(w,i,j,"%d",player_mat[i][j]);

                                if(player_mat[i][j]==3)
                                {
                                    wattron(w, COLOR_PAIR(1));
                                    mvwprintw(w,i,j,"X");
                                    wattroff(w, COLOR_PAIR(1));
                                }
                                if(player_mat[i][j]==4)
                                {
                                    mvwprintw(w,i,j,"X");
                                }
                            }

                        for(i=0;i<10;i++)
                            for(j=0;j<10;j++)
                                {
                                    if(pc_mat[i][j]==0 || pc_mat[i][j]==1 || pc_mat[i][j]==2)
                                        mvwprintw(w,i,j+12,"%c",254);
                                    if(pc_mat[i][j]==3)
                                    {
                                        wattron(w, COLOR_PAIR(1));
                                        mvwprintw(w,i,j+12,"X");
                                        wattroff(w, COLOR_PAIR(1));
                                    }
                                    if(pc_mat[i][j]==4)
                                        mvwprintw(w,i,j+12,"X");
                                }

                        //actualizare scor
                        mvwprintw(w,12,12,"PC Score :%d",pc_score);
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);

                        //actualizare salve
                        if(*charges>0)
                        {
                            mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);
                        }
                        else
                        {
                            mvwprintw(w,15,0,"Nu mai ai salve pentru d");    
                        }
                        wrefresh(w);

                    }

                    i=0;
                    j=0;
                    
                }
            }
            
            
            switch (key)
            {
                case KEY_UP:
                    {
                        i--;
                        if(i<0)
                            i=9;
                        break;
                    }
                case KEY_DOWN:
                    {
                        i++;
                        if(i>9)
                            i=0;
                        break;
                    }
                case KEY_LEFT:
                    {   
                        j--;
                        if(j<0)
                            j=9;
                        break;
                    }
                case KEY_RIGHT:
                    {
                        j++;
                        if(j>9)
                            j=0;
                        break;
                    }
            }
            if(pc_mat[ci][cj]==0 || pc_mat[ci][cj]==1 || pc_mat[ci][cj]==2)
                mvwprintw(w,ci,cj+12,"%c",254);
            if(pc_mat[ci][cj]==3)
                {
                    wattron(w, COLOR_PAIR(1));
                    mvwprintw(w,ci,cj+12,"X");
                    wattroff(w, COLOR_PAIR(1));
                }
            if(pc_mat[ci][cj]==4)
                mvwprintw(w,ci,cj+12,"X");

            mvwprintw(w,i,j+12,"+");//tinta

            wrefresh(w);
        }
        else //randul pc-ului
        {
            mvwprintw(w,11,0,"PC's turn                                    "); //resetez mesajul
            mvwprintw(w,12,12,"PC Score :%d",pc_score);

            wrefresh(w);

            //randomize hit point
            geni=rand()%10;
            genj=rand()%10;

            ok = 0;
            while(ok==0)
            {
                ok=1;
                if(player_mat[geni][genj]==1)
                {
                    pc_score++;
                    ok=0;// take another turn
                    player_mat[geni][genj]=3; // locul unde a lovit

                    
                    wattron(w, COLOR_PAIR(1));
                    mvwprintw(w,geni,genj,"X");
                    wattroff(w, COLOR_PAIR(1));
                    //ramane culoarea de background constant negru dupa ce a
                    //a facut prima colorare
                
                }
                else
                {
                    if(player_mat[geni][genj]==3 && player_mat[geni][genj]==4)//lovit deja
                    {
                        ok=0;
                        mvwprintw(w,geni,genj,"X");
                    }
                    else // playermat[geni][genj]==0 - a lovit un camp gol si care nu a mai fost lovit 
                    {
                        player_mat[geni][genj]=4; // locul unde a lovit
                        mvwprintw(w,geni,genj,"X"); // X = locul unde a lovit
                    }
                    
                    
                }
                sleep(1);
                
                wrefresh(w);
                //randomize hit point again
                geni=rand()%10;
                genj=rand()%10;
            }

            //se trmina randul pc-ului

            mvwprintw(w,11,0,"                                    "); //resetez mesajul
            mvwprintw(w,12,12,"PC Score :%d",pc_score);
            mvwprintw(w,11,0,"PC's turn ended. Press any key to take your turn");

            turn = 1;
        }
        
    }

    //wining condition
    if(player_score==20)
    {
        wclear(w);
        mvwprintw(w,0,0,"Ai Castigat! Press any key to end");
        wrefresh(w);
        wgetch(w);//astept un input a sa termin jpcul
        *enable=0;//dezactivez load-ul
    }
    else
        if(pc_score==20)
        {
            wclear(w);
            mvwprintw(w,0,0,"Calculatorul a castigat. Press any key to end");
            wrefresh(w);
            wgetch(w);//astept un input a sa termin jpcul
            *enable=0;//dezactivez load-ul
        }
        else // nu s-a terminat jocul ; slavez starea actuala
        {
            for(i=0;i<10;i++)
                for(j=0;j<10;j++)
                    cp_player_mat[i][j]=player_mat[i][j];

            for(i=0;i<10;i++)
                for(j=0;j<10;j++)
                    cp_pc_mat[i][j]=pc_mat[i][j];

            *cp_player_score=player_score;
            *cp_pc_score=pc_score;

        }
        
    
    delwin(w);
    endwin();
}

void loadgame(WINDOW *w, int cp_player_mat[10][10], int cp_pc_mat[10][10], int *cp_player_score, int *cp_pc_score, int *charges,int *enable)
{

    int i,j,key,ci,cj,d;
    int turn=1; // -1 pt randul pc-ului ; 1 pt randul player-ului
    int geni,genj;
    int ok;
    int player_score,pc_score;

    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);

    srand(time(NULL));
    initscr();

    w=newwin(100,100,1,1);

    wrefresh(w);

    player_score=*cp_player_score;
    pc_score=*cp_pc_score;

    //afiseaza matricile primite ca parametru si scorul
    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
        {
            if(cp_player_mat[i][j]==3)
            {
                wattron(w, COLOR_PAIR(1));
                mvwprintw(w,i,j,"X");
                wattroff(w, COLOR_PAIR(1));
            }
            else
            {
                if(cp_player_mat[i][j]==4)
                    mvwprintw(w,i,j,"X");
                else
                {
                    mvwprintw(w,i,j,"%d",cp_player_mat[i][j]);
                }
                
            }
            
        }

    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
        {
            if(cp_pc_mat[i][j]==0 || cp_pc_mat[i][j]==1 || cp_pc_mat[i][j]==2)
                mvwprintw(w,i,j+12,"%c",254);
            if(cp_pc_mat[i][j]==3)
            {
                wattron(w, COLOR_PAIR(1));
                mvwprintw(w,i,j+12,"X");
                wattroff(w, COLOR_PAIR(1));
            }
            if(cp_pc_mat[i][j]==4)
                mvwprintw(w,i,j+12,"X");
        }

    // +12 pt a parcurge vizual pc_mat

    i=0;
    j=0;

    if(*charges>0)
    {
        mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);
    }
    else
    {
        mvwprintw(w,15,0,"Nu mai ai salve pentru d");    
    }
    wrefresh(w);

    mvwprintw(w,i,j+12,"+");
    keypad(w,TRUE);//enable keyboard input
    
    mvwprintw(w,11,0,"Player's turn                                       ");
    //tb sa schimb pozitia textului
    mvwprintw(w,12,0,"P1 Score :%d",player_score);
    mvwprintw(w,12,12,"PC Score :%d",pc_score);

    while((key=wgetch(w))!='q' && player_score < 20 && pc_score < 20)
    {
        if(*charges>0)
        {
            mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);
        }
        else
        {
            mvwprintw(w,15,0,"Nu mai ai salve pentru d");    
        }
        wrefresh(w);

        if(turn == 1) //randul jucatorului
        {
        
            mvwprintw(w,11,0,"Player's turn                                       ");
            //tb sa schimb pozitia textului
            mvwprintw(w,12,0,"P1 Score :%d",player_score);
            mvwprintw(w,12,12,"PC Score :%d",pc_score);
            wrefresh(w);
            ci=i;
            cj=j;

            if (key==10)
            {
                if(cp_pc_mat[i][j]==1)
                {
                    player_score++;
                    mvwprintw(w,11,0,"lovit                                   ");
                    mvwprintw(w,12,0,"P1 Score :%d",player_score);

                    cp_pc_mat[i][j]=3; // 3 == casuta aleasa si barca lovita
                    //barca lovita o reprezinti altfe

                    wrefresh(w);
                }
                else
                    if(cp_pc_mat[i][j]==3 || cp_pc_mat[i][j]==4)
                    {
                        mvwprintw(w,11,0,"deja lovit                          ");
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);
                    }
                    else
                    {
                        cp_pc_mat[i][j]=4; // 4 == casuta aleasa si barca ratata
                        //barca ratata o reprezinti diferit

                        mvwprintw(w,11,0,"ratat, press any key for the pc turn            ");
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);
                        turn = -1;
                    }
                
            }
//destroy in advance
            else
            {
                if(key=='d' && *charges>0)
                {
                    d=destory_in_advance(w,cp_player_mat,cp_pc_mat,&player_score,&pc_score);
                    if(d==0)
                    {
                        mvwprintw(w,13,0,"nu se poate folosi functia destroy in advance, press any key to continue");
                        wrefresh(w);
                        mvwprintw(w,13,0,"                                                                        ");
                        wrefresh(w);
                        wgetch(w);
                    }
                    else
                    {
                        *charges=*charges-1;
                        for(i=0;i<10;i++)
                            for(j=0;j<10;j++)
                            {
                                mvwprintw(w,i,j,"%d",cp_player_mat[i][j]);

                                if(cp_player_mat[i][j]==3)
                                {
                                    wattron(w, COLOR_PAIR(1));
                                    mvwprintw(w,i,j,"X");
                                    wattroff(w, COLOR_PAIR(1));
                                }
                                if(cp_player_mat[i][j]==4)
                                {
                                    mvwprintw(w,i,j,"X");
                                }
                            }

                        for(i=0;i<10;i++)
                            for(j=0;j<10;j++)
                            {
                                if(cp_pc_mat[i][j]==0 || cp_pc_mat[i][j]==1 || cp_pc_mat[i][j]==2)
                                    mvwprintw(w,i,j+12,"%c",254);
                                if(cp_pc_mat[i][j]==3)
                                {
                                    wattron(w, COLOR_PAIR(1));
                                    mvwprintw(w,i,j+12,"X");
                                    wattroff(w, COLOR_PAIR(1));
                                }
                                if(cp_pc_mat[i][j]==4)
                                    mvwprintw(w,i,j+12,"X");
                            }

                        //actualizare scor
                        mvwprintw(w,12,12,"PC Score :%d",pc_score);
                        mvwprintw(w,12,0,"P1 Score :%d",player_score);

                        //actualizare salve
                        if(*charges>0)
                        {
                            mvwprintw(w,15,0,"Ai %d salve pentru d",*charges);
                        }
                        else
                        {
                            mvwprintw(w,15,0,"Nu mai ai salve pentru d");    
                        }
                        wrefresh(w);

                        
                    }
                    i=0;
                    j=0;
                    
                }
            }
            
            switch (key)
            {
                case KEY_UP:
                    {
                        i--;
                        if(i<0)
                            i=9;
                        break;
                    }
                case KEY_DOWN:
                    {
                        i++;
                        if(i>9)
                            i=0;
                        break;
                    }
                case KEY_LEFT:
                    {   
                        j--;
                        if(j<0)
                            j=9;
                        break;
                    }
                case KEY_RIGHT:
                    {
                        j++;
                        if(j>9)
                            j=0;
                        break;
                    }
            }
            
            if(cp_pc_mat[ci][cj]==0 || cp_pc_mat[ci][cj]==1 || cp_pc_mat[ci][cj]==2)
                mvwprintw(w,ci,cj+12,"%c",254);
            if(cp_pc_mat[ci][cj]==3)
            {
                wattron(w, COLOR_PAIR(1));
                mvwprintw(w,ci,cj+12,"X");
                wattroff(w, COLOR_PAIR(1));
            }
            if(cp_pc_mat[ci][cj]==4)
                mvwprintw(w,ci,cj+12,"X");

            mvwprintw(w,i,j+12,"+");//tinta

            wrefresh(w);
        }
        else //randul pc-ului
        {
            mvwprintw(w,11,0,"PC's turn                                    "); //resetez mesajul
            mvwprintw(w,12,12,"PC Score :%d",pc_score);
            wrefresh(w);

            //randomize hit point
            geni=rand()%10;
            genj=rand()%10;

            ok = 0;
            while(ok==0)
            {
                ok=1;
                if(cp_player_mat[geni][genj]==1)
                {
                    pc_score++;
                    ok=0;// take another turn
                    cp_player_mat[geni][genj]=3; // locul unde a lovit

                    
                    wattron(w, COLOR_PAIR(1));
                    mvwprintw(w,geni,genj,"X");
                    wattroff(w, COLOR_PAIR(1));
                    //ramane culoarea de background constant negru dupa ce a
                    //a facut prima colorare
                
                }
                else
                {
                    if(cp_player_mat[geni][genj]==3 && cp_player_mat[geni][genj]==4)//lovit deja
                    {
                        ok=0;
                        mvwprintw(w,geni,genj,"X");
                    }
                    else // playermat[geni][genj]==0 - a lovit un camp gol si care nu a mai fost lovit 
                    {
                        cp_player_mat[geni][genj]=4; // locul unde a lovit
                        mvwprintw(w,geni,genj,"X"); // X = locul unde a lovit
                    }
                    
                    
                }
                sleep(1);
                
                wrefresh(w);
                //randomize hit point again
                geni=rand()%10;
                genj=rand()%10;
            }

            //se trmina randul pc-ului

            mvwprintw(w,11,0,"                                    "); //resetez mesajul
            mvwprintw(w,12,12,"PC Score :%d",pc_score);
            mvwprintw(w,11,0,"PC's turn ended. Press any key to take your turn");
            
            //wgetch(w);

            turn = 1;
        }
        
    }

    //wining condition
    if(player_score==20)
    {
        wclear(w);
        mvwprintw(w,0,0,"Ai Castigat! Press any key to end");
        wrefresh(w);
        wgetch(w);//astept un input a sa termin jpcul
        *enable=0;//dezactivez load-ul
    }
    else
    {
        if(pc_score==20)
        {
            wclear(w);
            mvwprintw(w,0,0,"Calculatorul a castigat. Press any key to end");
            wrefresh(w);
            wgetch(w);//astept un input a sa termin jpcul
            *enable=0;//dezactivez load-ul
        }
        else //salvez variabilele
        {
            *cp_player_score=player_score;
            *cp_pc_score=pc_score;
        }
    }   
    
    delwin(w);
    endwin();
}

int destory_in_advance(WINDOW *w,int player_mat[10][10], int pc_mat[10][10], int *player_score, int *pc_score)
{
    int geni,genj;
    int i,j,k,playsc,pcsc,ok=1,cntpc=0,cntplay=0;
    srand(time(NULL));

    playsc=*player_score;
    pcsc=*pc_score;
    //vad daca este posibila folosirea acestei functii
    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
        {
            if(player_mat[i][j]==1 || player_mat[i][j]==0)
            {
                cntplay++;
            }
            if(pc_mat[i][j]==1 || pc_mat[i][j]==2 || pc_mat[i][j]==0)
            {
                cntpc++;
            }
        }

    if(cntplay < 10 || cntpc < 10)
    {
        return 0; //nu se poate folosi aceasta functie
    }

    k=10;

    while(k>0)
    {
        geni=rand()%10;
        genj=rand()%10;
        if(pc_mat[geni][genj]==1)
        {
            playsc++;
            pc_mat[geni][genj]=3; // lovit
            k--;
        }
        else
        {
            if(pc_mat[geni][genj]==0)
            {
                pc_mat[geni][genj]=4; // ratat
                k--;
            }
        }
        
    }

    k=10;

    while(k>0)
    {
        geni=rand()%10;
        genj=rand()%10;
        if(player_mat[geni][genj]==1)
        {
            pcsc++;
            player_mat[geni][genj]=3;
            k--;
        }
        else
        {
            if(player_mat[geni][genj]==0)
            {
                player_mat[geni][genj]=4;
                k--;
            }
        }
        
    }

    *player_score=playsc;
    *pc_score=pcsc;

    return 1;
}


void menu(WINDOW *w, char *file)
{

    char buttons[3][7] = { "New", "Resume", "Quit" };
    int i, key,charges;
    int height, width, start_y, start_x;
    int cp_player_mat[10][10],cp_pc_mat[10][10];
    int cp_player_score;
    int cp_pc_score;
    int enable;
    height=100;
    width=100;
    start_y=1;
    start_x=1;
    charges=2;
    enable=0; //load not enabled at start of the game

    initscr();
    w=newwin(height,width,start_y,start_x);//fereastra noua

    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);

    wrefresh(w);

    for(i=0;i<3;i++)
    {
        if(i==0)
        {
            wattron(w,A_STANDOUT); // highlight first item
        }
        else
        {
            wattroff(w,A_STANDOUT);
        }
        mvwprintw(w,i+1,2,"%s",buttons[i]);
    }

    mvwprintw(w,5,0,"Miscari speciale : \nDestroy in advance : d");

    wrefresh(w); // update screen

    i=0;
    noecho();//disable writing
    keypad(w,TRUE);//enable keyboard input
    curs_set(0);//hide cursor

    //get input
    while(key=wgetch(w))
    {
        mvwprintw(w,5,0,"Miscari speciale : \nDestroy in advance : d");
        mvwprintw(w,i+1,2,"%s",buttons[i]);

        if (key==10 && i==2)
            break;
        if (key==10 && i==0)
        {
            enable=1;
            newgame(w,file,cp_player_mat,cp_pc_mat,&cp_player_score,&cp_pc_score,&charges,&enable);
            wclear(w);
            for(i=0;i<3;i++)
            {
                mvwprintw(w,i+1,2,"%s",buttons[i]);
            }
            mvwprintw(w,5,0,"Miscari speciale : \nDestroy in advance : d");
            wrefresh(w);
        }
        if (key==10 && i==1 && enable==1)
        {
            loadgame(w,cp_player_mat,cp_pc_mat,&cp_player_score,&cp_pc_score,&charges,&enable);
            wclear(w);
            for(i=0;i<3;i++)
            {
                mvwprintw(w,i+1,2,"%s",buttons[i]);
            }
            mvwprintw(w,5,0,"Miscari speciale : \nDestroy in advance : d");
            wrefresh(w);
        }

        switch (key)
        {
        case KEY_UP:
                    i--;
                    if(i<0)
                    {
                        i=2;
                    }
                    break;
        case KEY_DOWN:
                    i++;
                    if(i>2)
                    {
                        i=0;
                    }
                    break;
        }

        //highlight the next item
        wattron(w,A_STANDOUT);
        mvwprintw(w,i+1,2,"%s",buttons[i]);
        wattroff(w,A_STANDOUT);
    }
    delwin(w);
    endwin();
}

int ok(int m[10][10], int dir, int pozi, int pozj, int dim)
{
    int i,j,k;
    int s,f;
    if(dir == 0) // vertical i ct, j modif
    {
        i=pozi;
        if( (10-pozj) < dim ) //nu intra in tabla
        {
            return 0; //fals
        }

        for(j=pozj;j<pozj+dim;j++)
        {
            if(m[i][j] != 0)
            {
                return 0;
            }
        }
        return 1;
        
        
    }
    if(dir == 1) // orizontal i modif, j ct
    {

        j=pozj;
        if( (10-pozi) < dim ) //nu intra in tabla
        {
            return 0; //fails
        }

        for(i=pozi;i<pozi+dim;i++)
        {
            if(m[i][j] != 0) 
            {
                return 0;
            }
        }
        return 1;
        
    }
    
}

void place(int m[10][10], int dir, int pozi, int pozj, int dim)
{
    int i,j;
    if(dir == 0 ) //orizontal i ct, j modif
    {
        // 1 = ship; 2 = vicinity
        m[pozi][pozj]=1;
        if(pozi==0)
        {
            if(pozj==0)
            {
                m[1][0]=2;
            }
            else
            {
                m[pozi][pozj-1]=2;
                m[pozi+1][pozj-1]=2;
                m[pozi+1][pozj]=2;
            }
        }
        else
        {
            if (pozi==9)
            {
                if(pozj==0)
                {
                    m[8][0]=2;
                }
                else
                {
                    m[pozi][pozj-1]=2;
                    m[pozi-1][pozj-1]=2;
                    m[pozi-1][pozj]=2;
                }
                
            }
            else
            {
                if(pozj==0)
                {
                    m[pozi-1][0]=2;
                    m[pozi+1][0]=2;
                }
                else
                {
                    m[pozi][pozj]=1;
                    m[pozi-1][pozj]=2;
                    m[pozi-1][pozj-1]=2;
                    m[pozi][pozj-1]=2;
                    m[pozi+1][pozj-1]=2;
                    m[pozi+1][pozj]=2;
                }
            }
        }

        for(j=pozj+1;j<pozj+dim-1;j++)
        {
            m[pozi][j]=1;
            if(pozi==0)
            {
                m[pozi+1][j]=2;
            }
            else
            {
                if(pozi==9)
                {
                    m[pozi-1][j]=2;
                }
                else
                {
                    m[pozi-1][j]=2;
                    m[pozi+1][j]=2;
                }
                
            }
            
        }

        j=pozj+dim-1;
        m[pozi][j]=1;

        if(pozi==0)
        {
            if(j==9)
            {
                m[1][9]=2;
            }
            else
            {
                m[pozi][j+1]=2;
                m[pozi+1][j+1]=2;
                m[pozi+1][j]=2;
            }
        }
        else
        {
            if (pozi==9)
            {
                if(j==9)
                {
                    m[8][9]=2;
                }
                else
                {
                    m[pozi-1][j]=2;
                    m[pozi-1][j+1]=2;
                    m[pozi][j+1]=2;
                }
                
            }
            else
            {
                if(j==9)
                {
                    m[pozi-1][9]=2;
                    m[pozi+1][9]=2;
                }
                else
                {
                    m[pozi-1][j]=2;
                    m[pozi-1][j+1]=2;
                    m[pozi][j+1]=2;
                    m[pozi+1][j+1]=2;
                    m[pozi+1][j]=2; 
                }
                
                
            }
        }

    }

    else // vertical i modif, j ct
    {
        m[pozi][pozj]=1;
        if (pozj==0)
        {
            if(pozi==0)
            {
                m[pozi][pozj+1]=2;
            }
            else
            {
                m[pozi-1][pozj]=2;
                m[pozi-1][pozj+1]=2;
                m[pozi][pozj+1]=2;
            }
        }
        else
        {
            if(pozj==9)
            {
                if(pozi==0)
                {
                    m[pozi][pozj-1]=2;
                }
                else
                {
                    m[pozi-1][pozj]=2;
                    m[pozi-1][pozj-1]=2;
                    m[pozi][pozj-1]=2;
                }
                
            }
            else
            {
                if(pozi==0)
                {
                    m[pozi][pozj-1]=2;
                    m[pozi][pozj+1]=2;
                }
                else
                {
                    m[pozi][pozj+1]=2;
                    m[pozi-1][pozj+1]=2;
                    m[pozi-1][pozj]=2;
                    m[pozi-1][pozj-1]=2;
                    m[pozi][pozj-1]=2; 
                }
                
            }
            
        }

        for(i=pozi+1;i<pozi+dim-1;i++)
        {
            m[i][pozj]=1;
            if(pozj==0)
            {
                m[i][pozj+1]=2;
            }
            else
            {
                if(pozj==9)
                {
                    m[i][pozj-1]=2;
                }
                else
                {
                    m[i][pozj-1]=2;
                    m[i][pozj+1]=2;
                    
                }
                
            }
            
        }

        i=pozi+dim-1;

        m[i][pozj]=1;

        if(pozj==0)
        {
            if(i==9)
            {
                m[i][pozj+1]=2;
            }
            else
            {
                m[i+1][pozj]=2;
                m[i+1][pozj+1]=2;
                m[i][pozj+1]=2;
            }
        }
        else
        {
            if(pozj==9)
            {
                if(i==9)
                {
                    m[i][pozj-1]=2;
                }
                else
                {
                    m[i][pozj-1]=2;
                    m[i+1][pozj-1]=2;
                    m[i+1][pozj]=2;
                }
            }
            else
            {
                if(i==9)
                {
                    m[i][pozj-1]=2;
                    m[i][pozj+1]=2;
                }
                else
                {
                    m[i][pozj-1]=2;
                    m[i+1][pozj-1]=2;
                    m[i+1][pozj]=2;
                    m[i+1][pozj+1]=2;
                    m[i][pozj+1]=2;
                    
                }
                
            }
            
        }
        
    }
    
}

void pcmat(int m[10][10])
{
    int i,j,geni,genj,dir,pas;
    int ships[5]={0,4,3,2,1};
    srand(time(NULL));

    //init mat

    for(i=0;i<10;i++)
    {
        for(j=0;j<10;j++)
        {
            m[i][j]=0;
        }
    }

    //place ships

    for(i=4;i>0;i--)
    {
        while (ships[i]>0)
        {
            //randomize the start of the ship
            geni=rand()%10;
            genj=rand()%10;
            if(m[geni][genj]==0)
            {
                dir = rand()%2;
                //vertical = 0 ; horizontal = 1

                if(ok(m,dir,geni,genj,i))
                {
                    place(m,dir,geni,genj,i);
                    ships[i]--;
                }
                //esle
                //randomize again
            }        
        }
    }
}

void playermat(int m[10][10], char *file)
{
    FILE* fin=fopen(file,"r");

    char lin[201];
    int i,j;

    i=0;
    while(fgets(lin,201,fin))
    {
        for(j=0;j<strlen(lin);j++)
        {
            if(lin[j]=='0')
            {
                m[i][j]=0;
            }
            if(lin[j]=='1')
            {
                m[i][j]=1;
            }
        }
        i++;
    }
}

int main(int argc, char **argv)
{
    FILE* fin=fopen(argv[1],"r");

    char file[100];
    WINDOW *w;
    WINDOW *gamew;
    start_color();

    if(!fin)
    {
        printf("Nu s-au dat argumente de comanda sau fisierul e greit scris sau nu exista\n");
        return 1;
    }
    strcpy(file,argv[1]);
    menu(w, file);
    return 0;
}