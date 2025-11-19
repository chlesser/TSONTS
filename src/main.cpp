#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

using namespace std;

enum resistance {
    NEUTRAL,
    RESISTANT,
    IMMUNE,
    VULNERABLE,
};

struct attack {
    vector<pair<int, int>>  DamageDices; //first number, then faces
    vector<resistance>      damageResistance;
    int                     diceTypes;
    int                     damageBonus;
    int                     hitBonus;
    int                     numberPerTurn;

    attack() {
        damageBonus = 0;
        hitBonus = 0;
        numberPerTurn = 1;
        diceTypes = 1;
        DamageDices.push_back(pair(1, 6));
        damageResistance.push_back(NEUTRAL);
    }
};

int hitDrop = 5;
int damageBump = 10;

bool crit19;
bool crit18;

bool reroll1;
int brutalCriticals;

vector<attack> Attacks;
int attackCount;

int bonusTypes;
vector<int> bonusCount;
vector<int> bonusSize;

int trials = 10000;

bool updateTable;
/*
    stores as follows:
    Normal, Gamble Standard
    Normal, Gamble Advantage
    Normal, Gamble Disadvantage
*/
float table[23][6];
string conclusions[23];

void runSimulation()
{
    float tito[4] = { 1.0f, 0.5f, 0.0f, 2.0f };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 20); //this is our d20 roll

    int critrange = 20;
    if(crit19)
        critrange--;
    if(crit18)
        critrange--;
    for(int ac = 8; ac <= 30; ac++) {
        for(int column = 0; column < 6; column++) {
            int agg = 0;
            for(int i = 0; i < trials; i++) {
                bool brutalCriticalLeft = brutalCriticals > 0;
                bool hit = false; //we store these for our automatic attacks
                bool critFirst = false; 
                //let's get hits
                for(int atkCtr = 0; atkCtr < Attacks.size(); atkCtr++) {
                    for(int attack = 0; attack < Attacks.at(atkCtr).numberPerTurn; attack++) {
                        int dieRoll = distrib(gen);
                        //here's where things get messy based on column...
                        if(column >= 4) { //disadvantage
                            int randomNumber2 = distrib(gen);
                            if(randomNumber2 < dieRoll)
                                dieRoll = randomNumber2;

                        } else if (column >= 2) { //advantage 
                            int randomNumber2 = distrib(gen);
                            if(randomNumber2 > dieRoll)
                                dieRoll = randomNumber2;
                        }
                        //if we're a halfling
                        if(reroll1 && dieRoll == 1)
                            dieRoll = distrib(gen);
                        
                        int attackDamage = 0;

                        //crit check first!
                        if(dieRoll >= critrange) {
                            //UPDATE THIS WITH THE CRIT CODE
                            for(int dmgDiceIndex = 0; dmgDiceIndex < Attacks.at(atkCtr).diceTypes; dmgDiceIndex++) {
                                for(int dmgDiceCount = 0; dmgDiceCount < Attacks.at(atkCtr).DamageDices.at(dmgDiceIndex).first; dmgDiceCount++) {
                                    std::uniform_int_distribution<> tempDistr(1, Attacks.at(atkCtr).DamageDices.at(dmgDiceIndex).second);
                                    attackDamage += tempDistr(gen);
                                }
                                if(brutalCriticalLeft) {
                                    for(int dmgDiceCount = 0; dmgDiceCount < brutalCriticals; dmgDiceCount++) {
                                        std::uniform_int_distribution<> tempDistr(1, Attacks.at(atkCtr).DamageDices.at(dmgDiceIndex).second);
                                        attackDamage += tempDistr(gen);
                                    }
                                    brutalCriticalLeft = false;
                                }
                            }
                            attackDamage += Attacks.at(atkCtr).damageBonus;
                            //and if we're gambling
                            if(column % 2 == 1)
                                attackDamage += damageBump;
                            if(!hit)
                                critFirst = true;
                            hit = true;
                        } else {
                            //we got our die roll. add our hit bonus
                            dieRoll += Attacks.at(atkCtr).hitBonus;
                            if(column % 2 == 1)
                                dieRoll -= hitDrop;
                            
                            if(dieRoll >= ac) {
                                hit = true;
                                for(int dmgDiceIndex = 0; dmgDiceIndex < Attacks.at(atkCtr).diceTypes; dmgDiceIndex++) {
                                    for(int dmgDiceCount = 0; dmgDiceCount < Attacks.at(atkCtr).DamageDices.at(dmgDiceIndex).first; dmgDiceCount++) {
                                        std::uniform_int_distribution<> tempDistr(1, Attacks.at(atkCtr).DamageDices.at(dmgDiceIndex).second);
                                        attackDamage += tempDistr(gen);
                                    }
                                }
                                attackDamage += Attacks.at(atkCtr).damageBonus;
                                if(column % 2 == 1)
                                    attackDamage += damageBump;
                            }
                        }
                        agg += attackDamage;
                    }
                }
                //thought I was free. Onto bonus damage.
                if(hit) {
                    for(int j = 0; j < bonusTypes; j++) {
                        for(int die = 0; die < bonusCount.at(j); die++) {
                            std::uniform_int_distribution<> tempDistr(1, bonusSize.at(j));
                            agg += tempDistr(gen);
                        }
                    }
                }
                if(critFirst) {
                    for(int j = 0; j < bonusTypes; j++) {
                        for(int die = 0; die < bonusCount.at(j); die++) {
                            std::uniform_int_distribution<> tempDistr(1, bonusSize.at(j));
                            agg += tempDistr(gen);
                        }
                    }
                }
            }
            //we're finally out of one round of fighting...
            float avg = static_cast<float>(agg) / static_cast<float>(trials);
            avg = truncf(avg * 100.0f) / 100.0f;
            cout << avg << endl;
            table[ac - 8][column] = avg;
        }
        //my final line of code, I swear!!!!
        //now we have this column in entirety, we can make a decision for the player.
        //whether it is, on average, better to attack with the gamble of -5, +10 or whatever parameters were set
        //we ignore disadvantage, but lets look at average damage for 0, 1, 2, 3
        //0 -> Standard Attack              1 -> Gamble Attack
        //2 -> Advantage Attack             3 -> Gamble Advantage Attack
        //we have 3 cases...
        //  GWM is better always            (lower ac)     1 > 0 && 3 > 2
        //  GWM is worse always             (high ac)      0 > 1 && 2 > 3
        //  GWM is better w/ advantage      (middle ac)    0 > 1 && 3 > 2
        //  GWM got really confused                        1 > 0 && 2 > 3
        float StdAtk = table[ac - 8][0];
        float GmbAtk = table[ac - 8][1];
        float StdAtkAdv = table[ac - 8][2];
        float GmbAtkAdv = table[ac - 8][3];
        if(GmbAtk >= StdAtk && GmbAtkAdv >=StdAtkAdv) {
            conclusions[ac - 8] = "Gamble";
        } else if (StdAtk >= GmbAtk && StdAtkAdv >=GmbAtkAdv) {
            conclusions[ac - 8] = "Standard";
        } else if (StdAtk >= GmbAtk && GmbAtkAdv >= StdAtkAdv) {
            conclusions[ac - 8] = "With Advantage";
        } else {
            conclusions[ac - 8] = "WTF?";
        }
    }
}

void createOptionsView()
{
    ImGui::Begin("Settings");
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("General"))
        {
            ImGui::SeparatorText("Martials only club");
            ImGui::Text("Welcome to the D&D Gamble Optimizer!\nIf you are worried whether you should use Great Weapon Master/Sharpshooter\nagainst a particular Armor Class, this has you covered!\nYou can also use it as a general damage calculator\nagainst an AC by removing gamble penalties and buffs.");

            ImGui::SeparatorText("Gamble Stats");
            ImGui::InputInt("To Hit Penality", &hitDrop);
            ImGui::InputInt("Damage Bonus", &damageBump);

            ImGui::SeparatorText("Critical Range");
            ImGui::Checkbox("Crit on 19", &crit19);
            ImGui::Checkbox("Crit on 18", &crit18);

            ImGui::SeparatorText("Abilities");
            ImGui::Checkbox("Reroll 1s", &reroll1);
            ImGui::InputInt("Brutal Critical Die", &brutalCriticals);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Attacks"))
        {
            ImGui::SeparatorText("Bump those numbers up");
            ImGui::Text("Declare attacks here, and how many times per turn they are used.");
            ImGui::SeparatorText("Attacks");
            ImGui::InputInt("How Many Types of Attacks", &attackCount);
            if(attackCount < 0)
                attackCount = 0;
            
            if ((int)Attacks.size() < attackCount)
                Attacks.resize(attackCount);

            for(int i = 0; i < attackCount; i++) {
                ImGui::PushID(i);

                string titleString = "Attack " + to_string(i + 1);

                ImGui::SeparatorText(titleString.c_str());

                ImGui::Text("Number Per Turn");
                ImGui::InputInt("##num", &(Attacks.at(i).numberPerTurn));

                ImGui::Text("To Hit Bonus");
                ImGui::InputInt("##hit", &(Attacks.at(i).hitBonus));

                ImGui::Text("Damage Bonus");
                ImGui::InputInt("##dmg", &(Attacks.at(i).damageBonus));

                ImGui::Text("Dice Types");
                ImGui::InputInt("##type", &(Attacks.at(i).diceTypes));

                if ((int)(Attacks.at(i).DamageDices.size()) < Attacks.at(i).diceTypes)
                    Attacks.at(i).DamageDices.resize(Attacks.at(i).diceTypes);
                
                if ((int)(Attacks.at(i).damageResistance.size()) < Attacks.at(i).diceTypes)
                    Attacks.at(i).damageResistance.resize(Attacks.at(i).diceTypes);

                for(int j = 0; j < Attacks.at(i).diceTypes; j++) {
                    ImGui::PushID(j);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // Red text
                    string subtitleString = "Dice type " + to_string(j + 1);
                    ImGui::Text(subtitleString.c_str());
                    ImGui::PopStyleColor();

                    ImGui::Text("Number of Dice");
                    ImGui::InputInt("##subsize", &Attacks.at(i).DamageDices[j].first);

                    ImGui::Text("Dice Faces");
                    ImGui::InputInt("##subcount", &Attacks.at(i).DamageDices[j].second);
                    
                    const char* items[] = { "Neutral", "Resistance", "Immunity", "Vulnerability"};
                    static int item_selected_idx = 0; // Here we store our selection data as an index.

                    static ImGuiComboFlags flags = 0;
                    const char* combo_preview_value = items[item_selected_idx];
                    if (ImGui::BeginCombo("Resistance", combo_preview_value, flags))
                    {
                        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                        {
                            const bool is_selected = (item_selected_idx == n);
                            if (ImGui::Selectable(items[n], is_selected)) {
                                item_selected_idx = n;
                                Attacks.at(i).damageResistance[j] = resistance(n);
                                cout << Attacks.at(i).damageResistance[j] << endl;
                            }
                                

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::PopID();
                }

                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("1/Turn"))
        {
            ImGui::SeparatorText("One shot, one kill");
            ImGui::Text("Once off damage added to only one attack, like sneak attack or hunters mark");
            ImGui::SeparatorText("Weapon Dice");
            ImGui::InputInt("How Many Types of Dice", &bonusTypes);
            if(bonusTypes < 0)
                bonusTypes = 0;
            
            if ((int)bonusSize.size() < bonusTypes)
                bonusSize.resize(bonusTypes, 0);

            if ((int)bonusCount.size() < bonusTypes)
                bonusCount.resize(bonusTypes, 0);
            
            
            for(int i = 0; i < bonusTypes; i++) {
                ImGui::PushID(i);

                string addedstring = "Dice Set ";
                addedstring += to_string(i + 1);

                ImGui::SeparatorText(addedstring.c_str());

                ImGui::Text("Dice Size");
                ImGui::InputInt("##size", &bonusSize[i]);

                ImGui::Text("Dice Amount");
                ImGui::InputInt("##count", &bonusCount[i]);

                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Build Table"))
        {
            ImGui::SeparatorText("Put it together");
            ImGui::Text("More trials will give more accurate information!");
            ImGui::InputInt("Trials per Case", &trials);
            if(ImGui::Button("Build!"))
                runSimulation();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
void createTableView()
{
    ImGui::Begin("Output");

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("table1", 8, flags))
        {
            ImGui::TableSetupColumn("AC");

            ImGui::TableSetupColumn("Normal STD");
            ImGui::TableSetupColumn("Gamble STD");

            ImGui::TableSetupColumn("Normal ADV");
            ImGui::TableSetupColumn("Gamble ADV");

            ImGui::TableSetupColumn("Normal DIS");
            ImGui::TableSetupColumn("Gamble DIS");

            ImGui::TableSetupColumn("Gamble?");
            ImGui::TableHeadersRow();

            for (int row = 0; row < 23; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 7; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    
                    if(column == 0) {
                        string printedString = to_string(row + 8);
                        ImGui::Text(printedString.c_str());
                    } else {
                        ImGui::Text("%.2f", table[row][column - 1]);
                    }
                }
                ImGui::TableSetColumnIndex(7);
                ImGui::Text(conclusions[row].c_str());
            }
            ImGui::EndTable();
        }
    ImGui::End();
}
int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "To Sharpshot or Not to Sharpshot", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    gladLoadGL();

    glViewport(0, 0, 800, 800);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        createOptionsView();
        createTableView();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}