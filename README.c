#include <stdio.h>
struct Person {
    char name[50];
    int age;
};

/*@関数のプロトタイプ宣言をおこなうこと*/
void print_num(int *num_pointer);
void print_array(int *array_numbers,int index);
void print_struct(struct Person p);
void print_pointer(struct Person *p);



int main() {
    //１）ポインタの基本操作ができますか？
    int number = 50;
    int* num_pointer = &number;
    print_num(num_pointer);
    
    
    //２）ポインタを利用して配列を操作できますか？
    int array_numbers[5] = {10, 11, 12, 13, 14};
    print_array(array_numbers, 2);
    
    
    //３）関数間で構造体をやりとりできますか？
    struct Person person1 = { "Ichiro", 30 };
    print_struct(person1);
    
    
    //４）ポインタを利用して関数間で構造体をやりとりできますか？
    struct Person person2 = { "Jiro", 26 };
    print_pointer(&person2);
    
    
    return 0;
}

void print_num(int *num_pointer){
    printf("number is %d\n", *num_pointer);
}

void print_array(int *array_numbers, int index){
    printf("index %d is %d\n", index, *(array_numbers+index));
}

void print_struct(struct Person p) {
    printf("struct Name: %s\n", p.name);
    printf("struct Age: %d\n", p.age);
}

void print_pointer(struct Person *p) {
    printf("pointer Name: %s\n", p->name);
    printf("pointer Age: %d\n", p->age);
}
