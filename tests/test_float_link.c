/* tools/float_link.sh, le corps de `make float-link`, sur des objets
 * fabriques : il se teste par ce qu'il doit empecher, un appel a libm que
 * le lien sans -lm laisserait passer. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_float_link_work"
#define OUT DIR "/out.txt"
#define BIN DIR "/bin/replay"
#define MAIN_O DIR "/main.o"
#define HALF_O DIR "/half.o"
#define SINE_O DIR "/sine.o"
#define LINK "sh tools/float_link.sh cc '-std=c99' " BIN

/* Tampon statique, termine par NUL, ecrase a chaque appel. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Rend le code de sortie du script, relu depuis un fichier : le statut brut
 * de system() n'est pas decodable sans <sys/wait.h>, hors C99. */
static int run_link(const char *objects) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, LINK " %s > " OUT " 2>&1; echo $? > " DIR "/code", objects);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(DIR "/main.c", "int half(int x);\nint main(void) { return half(2) - 1; }\n");
    write_file(DIR "/half.c", "int half(int x) { return x / 2; }\n");
    write_file(DIR "/sine.c",
               "#include <math.h>\n"
               "volatile double arg = 1.0;\n"
               "int half(int x) { return x / 2 + (int)sin(arg); }\n");
    assert(system("cc -std=c99 -c -o " MAIN_O " " DIR "/main.c") == 0);
    assert(system("cc -std=c99 -c -o " HALF_O " " DIR "/half.c") == 0);
    assert(system("cc -std=c99 -fno-builtin -c -o " SINE_O " " DIR "/sine.c") == 0);
}

static void test_integer_objects_link_without_libm(void) {
    setup();
    assert(run_link(MAIN_O " " HALF_O) == 0);
    assert(strstr(read_file(OUT), "float-link objets 2 non-resolus 0\n") != NULL);
    FILE *bin = fopen(BIN, "rb");
    assert(bin != NULL);
    fclose(bin);
}

static void test_libm_call_fails_and_is_named(void) {
    setup();
    if (system("cc -o " DIR "/probe " MAIN_O " " SINE_O " > /dev/null 2>&1") == 0) {
        printf("test_float_link: sin se lie sans -lm sur cet hote, cas saute\n");
        return;
    }
    assert(run_link(MAIN_O " " SINE_O) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "non-resolu sin\n") != NULL);
    assert(strstr(out, "float-link objets 2 non-resolus 1\n") != NULL);
}

static void test_no_object_is_an_error_not_a_pass(void) {
    setup();
    assert(run_link("") == 2);
}

int main(void) {
    test_integer_objects_link_without_libm();
    test_libm_call_fails_and_is_named();
    test_no_object_is_an_error_not_a_pass();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_link: all tests passed\n");
    return 0;
}
