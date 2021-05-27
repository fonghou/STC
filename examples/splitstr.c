#include <stc/csview.h>
#include <stc/cvec.h>

void print_split(csview str, csview sep)
{
    csview token = csview_first_token(str, sep);
    for (;;) {
        // print non-null-terminated csview
        printf("\t\"%.*s\"\n", csview_ARG(token));
        if (csview_end(&token).ref == csview_end(&str).ref) break;
        token = csview_next_token(str, sep, token);
    }
}

using_cvec_str();

cvec_str string_split(csview str, csview sep)
{
    cvec_str vec = cvec_str_init();
    csview token = csview_first_token(str, sep);
    for (;;) {
        cvec_str_push_back(&vec, cstr_from_v(token));
        if (csview_end(&token).ref == csview_end(&str).ref) break;
        token = csview_next_token(str, sep, token);
    }
    return vec;
}

int main()
{
    puts("Output from print_split():");
    print_split(c_lit("//This is a//double-slash//separated//string"), c_lit("//")); puts("");
    print_split(c_lit("This has no matching separator"), c_lit("xx")); puts("");

    puts("Output from string_split():");
    cstr string = cstr_lit("Split,this,,string,now,");
    cvec_str vec = string_split(c_sv(string), c_lit(","));

    c_fordefer (cvec_str_del(&vec), cstr_del(&string))
        c_foreach (i, cvec_str, vec)
            printf("\t\"%s\"\n", i.ref->str);
}