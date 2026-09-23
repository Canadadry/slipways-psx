/* tools/frames_ledger.sh, le corps de `make frames-ledger`, sur un depot git
 * fabrique : il se teste par ce qu'il doit empecher, un re-baseline sans
 * rapport ni ligne de releve qui passerait pour conforme. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_frames_ledger_work"
#define REPO DIR "/repo"
#define LEDGER DIR "/ledger.txt"
#define PROBE DIR "/probe_out.txt"
#define OUT DIR "/out.txt"
#define SCRIPT "sh tools/frames_ledger.sh " REPO " " LEDGER " " PROBE " reports"
#define GIT "git -C " REPO " "

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

static void sh(const char *cmd) {
    assert(system(cmd) == 0);
}

/* Commite tout l'arbre de travail et rend l'empreinte complete du commit,
 * dans un tampon statique propre a chaque slot. */
static const char *commit(int slot, const char *message) {
    static char hashes[8][64];
    char cmd[512];
    assert(slot >= 0 && slot < 8);
    snprintf(cmd, sizeof cmd, GIT "add -A && " GIT "commit -q -m '%s' && " GIT "rev-parse HEAD > " DIR "/head", message);
    sh(cmd);
    snprintf(hashes[slot], sizeof hashes[slot], "%.40s", read_file(DIR "/head"));
    return hashes[slot];
}

/* Rend le code de sortie du script, relu depuis un fichier : le statut brut
 * de system() n'est pas decodable sans <sys/wait.h>, hors C99. */
static int run_ledger(void) {
    sh(SCRIPT " > " OUT " 2>&1; echo $? > " DIR "/code");
    return atoi(read_file(DIR "/code"));
}

static const char *base, *pose, *divergence;

static void write_ledger(const char *lines) {
    char text[2048];
    snprintf(text, sizeof text, "# registre\nbase %.7s\n%s", base, lines);
    write_file(LEDGER, text);
}

static void write_report(const char *step, const char *divergent) {
    char path[256], text[512];
    snprintf(path, sizeof path, REPO "/reports/%s.txt", step);
    snprintf(text, sizeof text, "rapport de divergence, etape %s\n%s : DIVERGENT\n  premiere frame divergente : 3\n"
                                "c.json : conforme (5 frames)\n", step, divergent);
    write_file(path, text);
    snprintf(path, sizeof path, REPO "/reports/%s-attribution.txt", step);
    write_file(path, "attribution\n");
}

/* base : a.json.frames.hash ; pose : ajout de b ; divergence X1 : a reecrit,
 * rapport et attribution dans le meme commit. */
static void setup(void) {
    sh("rm -rf " DIR " && mkdir -p " REPO "/reports");
    sh(GIT "init -q && " GIT "config user.email t@example.invalid && " GIT "config user.name test"
           " && " GIT "config commit.gpgsign false");
    write_file(PROBE, "# probe\n## a literals\n0.001 0x0000.0041\n## b rounding\nspr(-0.5,4) -> 6,4\n");
    write_file(REPO "/a.json.frames.hash", "0\n");
    write_file(REPO "/c.json.frames.hash", "0\n");
    base = commit(0, "base");
    write_file(REPO "/b.json.frames.hash", "0\n");
    pose = commit(1, "pose");
    write_file(REPO "/a.json.frames.hash", "1\n");
    write_report("X1", "a.json");
    divergence = commit(2, "divergence");
}

static void write_right_ledger(void) {
    char lines[1024];
    snprintf(lines, sizeof lines, "%.9s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n", pose, divergence);
    write_ledger(lines);
}

static void test_right_ledger_passes(void) {
    setup();
    write_right_ledger();
    assert(run_ledger() == 0);
    assert(strstr(read_file(OUT), "frames-ledger commits 2 poses 1 divergences 1 erreurs 0\n") != NULL);
}

static void test_commit_missing_from_ledger_fails(void) {
    setup();
    char lines[512];
    snprintf(lines, sizeof lines, "%s | X1 | divergence | a | 0.001 0x0000.0041\n", divergence);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "absent du registre") != NULL);
}

static void test_ledger_commit_not_touching_references_fails(void) {
    setup();
    write_file(REPO "/other.txt", "x\n");
    const char *other = commit(3, "other");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n%s | P1 | pose\n",
             pose, divergence, other);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "ne touche aucun *.frames.hash") != NULL);
}

static void test_pose_that_rewrites_fails(void) {
    setup();
    write_file(REPO "/c.json.frames.hash", "1\n");
    write_file(REPO "/d.json.frames.hash", "0\n");
    const char *disguised = commit(3, "disguised");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n%s | P1 | pose\n",
             pose, divergence, disguised);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "pose qui reecrit c.json.frames.hash") != NULL);
}

static void test_divergence_without_report_fails(void) {
    setup();
    write_file(REPO "/c.json.frames.hash", "1\n");
    const char *bare = commit(3, "bare");
    write_report("X2", "c.json");
    commit(4, "late report");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n"
                                  "%s | X2 | divergence | b | spr(-0.5,4) -> 6,4\n", pose, divergence, bare);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "rapport absent du commit : reports/X2.txt") != NULL);
}

static void test_report_not_covering_rewritten_reference_fails(void) {
    setup();
    write_file(REPO "/a.json.frames.hash", "2\n");
    write_file(REPO "/c.json.frames.hash", "2\n");
    write_report("X2", "a.json");
    const char *partial = commit(3, "partial");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n"
                                  "%s | X2 | divergence | b | spr(-0.5,4) -> 6,4\n", pose, divergence, partial);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "reecrite hors rapport : c.json.frames.hash") != NULL);
}

static void test_report_declaring_an_unrewritten_reference_fails(void) {
    setup();
    write_file(REPO "/c.json.frames.hash", "2\n");
    write_report("X2", "c.json\nb.json : DIVERGENT\n  x\na.json");
    const char *extra = commit(3, "extra");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n"
                                  "%s | X2 | divergence | b | spr(-0.5,4) -> 6,4\n", pose, divergence, extra);
    write_ledger(lines);
    assert(run_ledger() == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "divergente non reecrite : a.json.frames.hash") != NULL);
    assert(strstr(out, "divergente non reecrite : b.json.frames.hash") != NULL);
}

static void test_probe_line_absent_or_in_another_block_fails(void) {
    setup();
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.002 0x0000.0083\n", pose, divergence);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "ligne de releve introuvable dans le bloc a") != NULL);
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | b | 0.001 0x0000.0041\n", pose, divergence);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "ligne de releve introuvable dans le bloc b") != NULL);
}

static void test_deleted_reference_fails(void) {
    setup();
    sh("rm " REPO "/b.json.frames.hash");
    const char *deleted = commit(3, "delete");
    char lines[1024];
    snprintf(lines, sizeof lines, "%s | P0 | pose\n%s | X1 | divergence | a | 0.001 0x0000.0041\n%s | P1 | pose\n",
             pose, divergence, deleted);
    write_ledger(lines);
    assert(run_ledger() == 1);
    assert(strstr(read_file(OUT), "non prevu par le registre") != NULL);
}

static void test_unknown_base_is_an_error_not_a_pass(void) {
    setup();
    write_file(LEDGER, "base 0123456789abcdef0123456789abcdef01234567\n");
    assert(run_ledger() == 2);
    assert(strstr(read_file(OUT), "git fetch --unshallow") != NULL && "base injoignable : le message doit nommer le remede");
    assert(strstr(read_file(OUT), "--deepen") != NULL);
}

static void test_not_a_git_repository_is_an_error(void) {
    setup();
    write_right_ledger();
    sh("rm -rf " REPO "/.git");
    assert(run_ledger() == 2);
}

int main(void) {
    test_right_ledger_passes();
    test_commit_missing_from_ledger_fails();
    test_ledger_commit_not_touching_references_fails();
    test_pose_that_rewrites_fails();
    test_divergence_without_report_fails();
    test_report_not_covering_rewritten_reference_fails();
    test_report_declaring_an_unrewritten_reference_fails();
    test_probe_line_absent_or_in_another_block_fails();
    test_deleted_reference_fails();
    test_unknown_base_is_an_error_not_a_pass();
    test_not_a_git_repository_is_an_error();
    sh("rm -rf " DIR);
    printf("test_frames_ledger: all tests passed\n");
    return 0;
}
