### Usage

```shell
hist insert [--file /path/to/file]
hist get [--hostname host] [--from timestamp] [--to timestamp] [--file /path/to/file] [--format hist-format]
```

### Refs

https://zetcode.com/db/sqlitec/
https://www.sqlite.org/rescode.html#done
https://dev.w3.org/libwww/Library/src/vms/getline.c
https://www.tutorialspoint.com/c_standard_library/c_function_strchr.htm
https://zsh.sourceforge.io/Doc/Release/Shell-Builtin-Commands.html
https://shapeshed.com/unix-fc/
https://zsh.sourceforge.io/Doc/Release/Shell-Builtin-Commands.html#Shell-Builtin-Commands

https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
https://linuxprograms.wordpress.com/2012/06/22/c-getopt_long-example-accessing-command-line-arguments/
https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c
https://github.com/mct/junkdrawer/blob/master/c/getopt_long-example.c
https://cfengine.com/blog/2021/optional-arguments-with-getopt-long/

---

- les données à proprement parler (data)
  - un array de bytes
- la taille de la str (len)
- la taille totale disponible dans le buffer (capacity)

```c
// concatenates src in target
str_cat(str target, str src);
// converts std string to str
str str_from(char *src, size_t size);
// converts a str to std string
// statically garantees that const char * is null terminated
char *str_to(str s);
// returns a copy of str
str str_cpy(str s);
// creates a new str
str str_new();
// deletes a str
str_del(str s);
```

le but étant d'avoir une api de str à la C++
immuable
sans faire de heap allocation pour chaque str
besoin d'un comportement similaire à un array/buffer ou l'on peu append un ou plusieurs elements (str_cat?)

autre structure de donnée comparable
Stream ? file buffer?
immuable
on copie l'entierté du contenu d'un fd dans ce buffer
un curseur gardant la position dans le buffer qui est auto incrémenté par les methodes de lecture et d'ecriture
une methode peek qui permet de regarder le byte suivant sans toucher au curseur, et qui retourne EOF en fin de fichier

Un buffer
mutable
capable de se redimensionner dynamiquement
methodes de facilitation de transfert de données 
pas de pointer arithmetic
plusieurs modes d'insertion
into
ajoute le target au début, écrase le preexistant,  allocation si dépassement de la capacity
append
ajoute à la suite, après le curseur? allocation si dépassement de la capacity
clear
zero'd whole capacity
