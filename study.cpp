#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <regex>
#include <cctype>

using namespace std;


/* =====================================================
   DATA STRUCTURES
===================================================== */

struct Fact {
    string term;
    string definition;
};


struct Question {
    string question;
    string correctAnswer;
    vector<string> choices;
};


/* =====================================================
   TRIM
===================================================== */

string trim(const string& text) {

    size_t start = 0;

    while (start < text.size() &&
           isspace(static_cast<unsigned char>(text[start]))) {
        start++;
    }


    size_t end = text.size();

    while (end > start &&
           isspace(static_cast<unsigned char>(text[end - 1]))) {
        end--;
    }


    return text.substr(start, end - start);
}


/* =====================================================
   REMOVE BULLET
===================================================== */

string cleanLine(string line) {

    line = trim(line);

    if (!line.empty()) {

        if (line[0] == '*' ||
            line[0] == '-') {

            /*
                Only remove a leading bullet.
                The actual separator is handled later.
            */

            if (line.size() > 1 &&
                isspace(static_cast<unsigned char>(line[1]))) {

                line = trim(line.substr(1));
            }
        }
    }

    return line;
}


/* =====================================================
   REMOVE TRAILING PUNCTUATION
===================================================== */

string cleanDefinition(string text) {

    text = trim(text);

    while (!text.empty() &&
           (text.back() == '.' ||
            text.back() == ',' ||
            text.back() == ';')) {

        text.pop_back();
        text = trim(text);
    }

    return text;
}


/* =====================================================
   CHECK FACT
===================================================== */

bool validFact(const string& term,
               const string& definition) {

    if (term.empty() ||
        definition.empty()) {
        return false;
    }

    if (definition.length() < 2) {
        return false;
    }

    return true;
}


/* =====================================================
   PARSE ONE DEFINITION LINE
===================================================== */

bool parseDefinitionLine(
    const string& original,
    Fact& result
) {

    string line = cleanLine(original);

    if (line.empty()) {
        return false;
    }


    /*
        Supported:

        Term - Meaning
        Term : Meaning
        Term – Meaning
        Term — Meaning
        Term = Meaning
        Term -> Meaning
        Term → Meaning
    */


    vector<string> separators = {
        "->",
        "→",
        " - ",
        " – ",
        " — ",
        " : ",
        " = "
    };


    for (const string& separator : separators) {

        size_t position =
            line.find(separator);


        if (position != string::npos) {

            string term =
                trim(line.substr(0, position));

            string definition =
                trim(
                    line.substr(
                        position + separator.length()
                    )
                );


            if (validFact(term, definition)) {

                result.term =
                    cleanDefinition(term);

                result.definition =
                    cleanDefinition(definition);

                return true;
            }
        }
    }


    /*
        Also support:

        Term is Meaning
        Term means Meaning
        Term refers to Meaning
        Term is defined as Meaning
    */


    vector<string> phrases = {
        " is defined as ",
        " refers to ",
        " means ",
        " is ",
        " are "
    };


    string lowerLine = line;

    transform(
        lowerLine.begin(),
        lowerLine.end(),
        lowerLine.begin(),
        [](unsigned char c) {
            return static_cast<char>(tolower(c));
        }
    );


    for (const string& phrase : phrases) {

        size_t position =
            lowerLine.find(phrase);


        if (position != string::npos) {

            string term =
                trim(line.substr(0, position));

            string definition =
                trim(
                    line.substr(
                        position + phrase.length()
                    )
                );


            if (validFact(term, definition)) {

                result.term =
                    cleanDefinition(term);

                result.definition =
                    cleanDefinition(definition);

                return true;
            }
        }
    }


    return false;
}


/* =====================================================
   EXTRACT FACTS
===================================================== */

vector<Fact> extractFacts(
    const vector<string>& lines
) {

    vector<Fact> facts;


    for (size_t i = 0;
         i < lines.size();
         i++) {

        Fact fact;


        if (parseDefinitionLine(lines[i], fact)) {

            facts.push_back(fact);

            continue;
        }


        /*
            Support:

            Term:
            Meaning

            Term -
            Meaning
        */

        string line =
            cleanLine(lines[i]);


        if (!line.empty() &&
            (line.back() == ':' ||
             line.back() == '-')) {

            string term =
                trim(
                    line.substr(
                        0,
                        line.length() - 1
                    )
                );


            if (i + 1 < lines.size()) {

                string definition =
                    cleanLine(lines[i + 1]);


                if (validFact(term, definition)) {

                    facts.push_back({
                        cleanDefinition(term),
                        cleanDefinition(definition)
                    });

                    i++;
                }
            }
        }
    }


    /* Remove duplicate facts */

    vector<Fact> uniqueFacts;


    for (const Fact& fact : facts) {

        bool duplicate = false;


        for (const Fact& existing : uniqueFacts) {

            if (
                existing.term == fact.term &&
                existing.definition == fact.definition
            ) {

                duplicate = true;
                break;
            }
        }


        if (!duplicate) {
            uniqueFacts.push_back(fact);
        }
    }


    return uniqueFacts;
}


/* =====================================================
   SHUFFLE
===================================================== */

template <typename T>
void shuffleVector(vector<T>& data) {

    static random_device rd;
    static mt19937 generator(rd());

    shuffle(
        data.begin(),
        data.end(),
        generator
    );
}


/* =====================================================
   GENERATE QUESTIONS
   QUESTIONS USE THE MEANING OF TERMS
===================================================== */

vector<Question> generateQuestions(
    const vector<Fact>& facts,
    int count
) {

    vector<Question> questions;


    if (facts.empty()) {
        return questions;
    }


    vector<string> templates = {

        "What does \"TERM\" mean?",

        "Which definition correctly describes \"TERM\"?",

        "Which statement gives the meaning of \"TERM\"?",

        "What is the correct meaning of \"TERM\"?",

        "Which choice best explains \"TERM\"?"
    };


    vector<Fact> shuffledFacts = facts;

    shuffleVector(shuffledFacts);


    for (int i = 0;
         i < count;
         i++) {


        const Fact& fact =
            shuffledFacts[
                i % shuffledFacts.size()
            ];


        string questionTemplate =
            templates[
                rand() % templates.size()
            ];


        size_t position =
            questionTemplate.find("TERM");


        if (position != string::npos) {

            questionTemplate.replace(
                position,
                4,
                fact.term
            );
        }


        Question question;

        question.question =
            questionTemplate;

        /*
            IMPORTANT:
            Correct answer is the meaning
            provided by the user.
        */

        question.correctAnswer =
            fact.definition;


        /*
            Wrong choices are other definitions
            from the user's material.
        */

        vector<string> wrongAnswers;


        for (const Fact& other : facts) {

            if (
                other.term != fact.term &&
                other.definition != fact.definition
            ) {

                wrongAnswers.push_back(
                    other.definition
                );
            }
        }


        shuffleVector(wrongAnswers);


        question.choices.push_back(
            fact.definition
        );


        for (const string& answer : wrongAnswers) {

            if (question.choices.size() >= 4) {
                break;
            }

            question.choices.push_back(answer);
        }


        /*
            Do NOT invent definitions.
            If the material has fewer than
            four meanings, use this neutral
            fallback.
        */

        while (question.choices.size() < 4) {

            question.choices.push_back(
                "Not provided in the given material."
            );
        }


        shuffleVector(question.choices);


        questions.push_back(question);
    }


    return questions;
}


/* =====================================================
   MAIN
===================================================== */

int main() {

    vector<string> lines;

    string line;


    cout << "Enter your terminology and meanings.\n";
    cout << "Example:\n";
    cout << "Photosynthesis - The process by which plants make food.\n";
    cout << "Chlorophyll: A green pigment found in plants.\n";
    cout << "Type END when finished.\n\n";


    while (getline(cin, line)) {

        if (line == "END" ||
            line == "end") {

            break;
        }

        lines.push_back(line);
    }


    vector<Fact> facts =
        extractFacts(lines);


    if (facts.size() < 2) {

        cout << "\nNot enough terminology definitions.\n";

        return 0;
    }


    int count;


    cout << "\nHow many questions? ";

    cin >> count;


    if (count < 1) {
        count = 1;
    }

    if (count > 50) {
        count = 50;
    }


    vector<Question> questions =
        generateQuestions(
            facts,
            count
        );


    cout << "\n===== GENERATED QUIZ =====\n\n";


    for (size_t i = 0;
         i < questions.size();
         i++) {

        cout << i + 1
             << ". "
             << questions[i].question
             << "\n";


        for (size_t j = 0;
             j < questions[i].choices.size();
             j++) {

            cout << "   "
                 << char('A' + j)
                 << ". "
                 << questions[i].choices[j]
                 << "\n";
        }


        cout << "\n";
    }


    return 0;
}
