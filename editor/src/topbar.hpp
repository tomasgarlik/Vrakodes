void emptyfunc();
stiffness = {
    .xpos=280,
    .ypos=STANDARTPICEHEIGHT,
    .text="20",
    .active=false,
    .size=55,
    .maxnum=90000
};
min_len = {
    .xpos=280,
    .ypos=STANDARTPICEHEIGHT*2,
    .text="80",
    .active=false,
    .size=55,
    .maxnum=100
};
damping = {
    .xpos=390,
    .ypos=STANDARTPICEHEIGHT,
    .text="80",
    .active=false,
    .size=40,
    .maxnum=1000
};
elastic_margin = {
    .xpos=450,
    .ypos=STANDARTPICEHEIGHT*2,
    .text="5",
    .active=false,
    .size=40,
    .maxnum=100
};
snap = {
    .title=createTextTexture(renderer, "Snap"),
    .ypos=STANDARTPICEHEIGHT,
    .xpos=440,
    .checked=true
};
mass = {
    .xpos=560,
    .ypos=STANDARTPICEHEIGHT,
    .text="20",
    .active=false,
    .size=60,
    .maxnum=100000
};
collide = {
    .title=createTextTexture(renderer, "Collide"),
    .ypos=STANDARTPICEHEIGHT+3,
    .xpos=630,
    .checked=true
};
self_collide = {
    .title=createTextTexture(renderer, "Self collide"),
    .ypos=(STANDARTPICEHEIGHT*2)+3,
    .xpos=600,
    .checked=false
};
friction = {
    .xpos=550,
    .ypos=STANDARTPICEHEIGHT*2,
    .text="20",
    .active=false,
    .size=40,
    .maxnum=200
};
group = {
    .functions={emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc},
    .textures={createTextTexture(renderer, "Group 1"), createTextTexture(renderer, "Group 2"), createTextTexture(renderer, "Group 3"), createTextTexture(renderer, "Group 4"), createTextTexture(renderer, "Group 5"), createTextTexture(renderer, "Group 6"), createTextTexture(renderer, "Group 7"), createTextTexture(renderer, "Group 8"), createTextTexture(renderer, "Group 9"), createTextTexture(renderer, "Group 10")},
    .lenght=10,
    .titletype="select",
    .topwidth=80,
    .xpos=710,
    .ypos=STANDARTPICEHEIGHT,
    .width=80,
    .dashable=true
};
mode = {
    .functions={emptyfunc, emptyfunc, emptyfunc, emptyfunc},
    .textures={createTextTexture(renderer, "Select"), createTextTexture(renderer, "Union"), createTextTexture(renderer, "ICJ creation"), createTextTexture(renderer, "fjdk")},
    .lenght=4,
    .titletype="select",
    .topwidth=120,
    .xpos=80,
    .ypos=STANDARTPICEHEIGHT,
    .width=100,
    .dashable=false
};
xray = {
    .title=createTextTexture(renderer, "X-ray"),
    .ypos=2,
    .xpos=80,
    .checked=false
};
attribute = {
    .functions={emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc, emptyfunc},
    .textures={createTextTexture(renderer, "--"), createTextTexture(renderer, "LB"), createTextTexture(renderer, "RB"), createTextTexture(renderer, "LF"), createTextTexture(renderer, "RF"), createTextTexture(renderer, "SS"), createTextTexture(renderer, "LS"), createTextTexture(renderer, "Up"), createTextTexture(renderer, "ICJ"), createTextTexture(renderer, "CJ"), createTextTexture(renderer, "CRJ")},
    .lenght=11,
    .titletype="select",
    .topwidth=40,
    .xpos=780,
    .ypos=STANDARTPICEHEIGHT*2,
    .width=40,
    .dashable=true
};
projection = {
    .functions={emptyfunc, emptyfunc, emptyfunc, emptyfunc},
    .textures={createTextTexture(renderer, "Perspective"), createTextTexture(renderer, "Orthografic")},
    .lenght=2,
    .titletype="select",
    .topwidth=120,
    .xpos=80,
    .ypos=STANDARTPICEHEIGHT*2,
    .width=120,
    .dashable=false
};
show_points = {
    .title=createTextTexture(renderer, "Show points"),
    .ypos=5,
    .xpos=1120,
    .checked=true
};
show_joints = {
    .title=createTextTexture(renderer, "Show joints"),
    .ypos=25,
    .xpos=1120,
    .checked=true
};
show_faces = {
    .title=createTextTexture(renderer, "Show faces"),
    .ypos=45,
    .xpos=1120,
    .checked=true
};
show_g1 = {
    .title=createTextTexture(renderer, "g1"),
    .ypos=5,
    .xpos=1240,
    .checked=true
};
show_g2 = {
    .title=createTextTexture(renderer, "g2"),
    .ypos=25,
    .xpos=1240,
    .checked=true
};
show_g3 = {
    .title=createTextTexture(renderer, "g3"),
    .ypos=45,
    .xpos=1240,
    .checked=true
};
show_g4 = {
    .title=createTextTexture(renderer, "g4"),
    .ypos=5,
    .xpos=1280,
    .checked=true
};
show_g5 = {
    .title=createTextTexture(renderer, "g5"),
    .ypos=25,
    .xpos=1280,
    .checked=true
};
show_g6 = {
    .title=createTextTexture(renderer, "g6"),
    .ypos=45,
    .xpos=1280,
    .checked=true
};
show_g7 = {
    .title=createTextTexture(renderer, "g7"),
    .ypos=5,
    .xpos=1320,
    .checked=true
};
show_g8 = {
    .title=createTextTexture(renderer, "g8"),
    .ypos=25,
    .xpos=1320,
    .checked=true
};
show_g9 = {
    .title=createTextTexture(renderer, "g9"),
    .ypos=45,
    .xpos=1320,
    .checked=true
};
show_g10 = {
    .title=createTextTexture(renderer, "g10"),
    .ypos=5,
    .xpos=1360,
    .checked=true
};
show_icj = {
    .title=createTextTexture(renderer, "ICJ"),
    .ypos=25,
    .xpos=1360,
    .checked=true
};