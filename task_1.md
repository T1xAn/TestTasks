
## Исходный код

```cpp
typedef struct list_s{

        struct list_s *next; /* NULL for the last item in a list */
        int data;
}
list_t;


int count_list_items(const list_t *head) {

         if (head->next) {
                   return count_list_items(head->next) + 1;
         } else {
                   return 1;
         }
}

    /* Inserts a new list item after the one specified as the argument.
          */
void insert_next_to_list(list_t *item, int data) {

         (item->next = malloc(sizeof(list_t)))->next = item->next;
         item->next->data = data;

}

    /* Removes an item following the one specificed as the argument.
          */

void remove_next_from_list(list_t *item) {
          if (item->next) {
                   free(item->next);
        item->next = item->next->next;
     }
}

         /* Returns item data as text.
          */

char *item_data(const list_t *list){

         char buf[12];
         sprintf(buf, "%d", list->data);
         return buf;
}
```

---

## Исправленный код

```cpp
typedef struct list_s{

        struct list_s *next; /* NULL for the last item in a list */
        int data;
}
list_t;


//В подсчёте количества элементов мне кажется что рекурсия лишняя, так как мы потеряем больше времени
//и памяти переключаясь между стеками и контекстами функций, чем просто прошлись бы по списку.
//Можно оставить указатель на head с const но придётся добавить ещё локальную переменную
int count_list_items(list_t* head) {

	int count = 0;

	while (head) {
		head = head->next;
		count++;
	}

	return count;
}

//В функции вставки нового элемента в лист нужно сделать проверку, есть ли на месте next какие либо данные,
//так же из за сложной конструкции происходит переопределение элемента листа и зацикливание указателем next
// на самого себя. Так же нужно добавить проверки на выделение памяти
void insert_next_to_list(list_t *item, int data) {	 
	 list_t* new_item = NULL;
	 item->next = new_item = (list_t*)malloc(sizeof(list_t));
	 if (new_item)
		 new_item->data = data;
	 else
		 // Сообщение об ошибке выделения памяти;
}

//В функции удаления нужно добавить проверку на то есть ли у следующего
//элемента ещё указатель на следующий что бы не допустить потерю памяти
void remove_next_from_list(list_t *item) {
          if (item->next) {
		     list_t * nextItem = item->next;
		     item->next = nextItem->next;// Если там пусто то присвоится NULL а если нет то правильное значение
             free(nextItem);
     }
}

//В данной функции лучше передавать буфер как параметр, 
//так как мы не можем гарантировать что в локальном буфере после возвращения данные будут верны
void item_data(const list_t* list, char* buf, size_t buf_size)
{
    snprintf(buf, buf_size, "%d", list->data);
}
```
