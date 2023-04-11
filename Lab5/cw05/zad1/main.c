#include <stdio.h>
#include <string.h>

enum OPTION{
    SEND,
    WRITE_DATE,
    WRITE_NAME
};

void print_correct_format();

void send_email(char * email_address, char * title, char * text);

void list_emails(enum OPTION option);

int main(int argc, char ** argv){
    if (argc != 2 && argc != 4){
        printf("Error: Correct number of arguments are one or four\n");
        print_correct_format();
        return -1;
    }

    enum OPTION option;
    char * email_address = NULL;
    char * title = NULL;
    char * text = NULL;

    if(argc == 2){
        if (strcmp(argv[1], "nadawca") == 0) option = WRITE_NAME;
        else if (strcmp(argv[1], "data") == 0) option = WRITE_DATE;
        else{
            printf("Error: Invalid option %s\n", argv[1]);
            print_correct_format();
            return -1;
        }
    }

    else{
        email_address = argv[1];
        title = argv[2];
        text = argv[3];
        option = SEND;
    }

    if(option == SEND) send_email(email_address, title, text);
    else list_emails(option);

    return 0;
}


void print_correct_format(){
    printf("Correct format:\n"
           "1) ./main arg  - write list of email sort by args\n"
           "2) ./main email_address, title, 'text' - send email to email_adress with title and text\n");
}

void send_email(char * email_address, char * title, char * text){

    char command[1024];
    snprintf(command, 1024, "mail -s %s %s" ,title, email_address);
    FILE * stream = popen(command, "w");

    fputs(text, stream);
    pclose(stream);
}

void list_emails(enum OPTION option){

    FILE * stream;

    switch (option) {
        case WRITE_DATE:
            stream = popen("echo | mail | tail -n+2 | head -n-2", "r");
            fgetc(stream);
            break;

        case WRITE_NAME:
            stream = popen("echo | mail | tail -n+2 | head -n-2| sort -k3", "r");
            break;
        default: break;
    }

    char buffer[1024];

    while (fgets(buffer, 1024, stream) != NULL) {
        printf("%s", buffer);
    }

    pclose(stream);

}