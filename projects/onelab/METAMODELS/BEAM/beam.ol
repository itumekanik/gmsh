MATERIAL.number(1, 3Material/0,"Name");
MATERIAL.valueLabels(0, "User defined", 1, "Steel", 2, "Wood", 3, "Alu");
YOUNG.number(, 3Material/, "Young modulus [GPa]");
POISSON.number(, 3Material/, "Poisson ratio []");
DENSITY.number(, 3Material/, "Mass density [kg/m3]");

OL.if(OL.get(MATERIAL) == 1)
  YOUNG.setValue(210);
  POISSON.setValue(0.3);
  DENSITY.setValue(7500);
OL.endif
OL.if(OL.get(MATERIAL) == 2)
  YOUNG.setValue(13);
  POISSON.setValue(0.45);
  DENSITY.setValue(900);
OL.endif
OL.if(OL.get(MATERIAL) == 3)
  YOUNG.setValue(70);
  POISSON.setValue(0.33);
  DENSITY.setValue(6000);
OL.endif
OL.if(OL.get(MATERIAL) == 0)
  YOUNG.setReadOnly(0);
  POISSON.setReadOnly(0);
  DENSITY.setReadOnly(0);
OL.endif

WEIGHT.radioButton(1, 3Material/0, "Account for weight");
LOOP.radioButton(0,,"Compute MNT diagram");

M.number(,9Results/,"Moment (M)");
N.number(,9Results/,"Traction (N)");
T.number(,9Results/,"Shear (Q)");
vmin.number(,9Results/,"min v(x)");
vmax.number(,9Results/,"max v(x)");

X.number(0.75, 1Geometry/,"Cut location");

OL.iftrue(LOOP)
X.setAttribute(Loop,3);
X.setAttribute(Graph,101000);
9Results/M.setAttribute(Graph,010000);
9Results/T.setAttribute(Graph,000100);
OL.else
  OL.if( OL.get(X, attrib.get(Loop)) == 3)
    X.setAttribute(Loop,0);
  OL.endif
X.setAttribute(Graph,0);
9Results/M.setAttribute(Graph,0);
9Results/T.setAttribute(Graph,0);
OL.endif

OL.if( OL.get(X, choices.index()) == 0 )
9Results/M.resetChoices();
9Results/T.resetChoices();
OL.endif

#1) Client Gmsh pour le maillage initial
Mesher.register(native);
Mesher.in(OL.get(Arguments/FileName).geo);
Mesher.out(OL.get(Arguments/FileName).msh);
Mesher.run(OL.get(Arguments/FileName).geo);
Mesher.frontPage(OL.get(Arguments/FileName).geo,
                 OL.get(Arguments/FileName).msh);

#X.range(0.15:OL.eval(0.99*OL.get(1Geometry/L))|15.00001);
OL.iftrue(LOOP)
X.range(0.10:OL.get(1Geometry/L)|10.00001);
OL.else
X.range(0.0:OL.get(1Geometry/L):O.1);
X.withinRange();
OL.endif

#2) Client ElmerGrid pour convertir le maillage pour Elmer
ElmerGrid.register(interfaced);
ElmerGrid.in(OL.get(Arguments/FileName).msh);
ElmerGrid.out(mesh);
ElmerGrid.run(14 2 OL.get(Arguments/FileName).msh -out mesh);

#3) Client Elmer pour lancer la simulation avec Elmer
Elmer.register(encapsulated);
Elmer.in(ELMERSOLVER_STARTINFO.ol, beam.sif.ol);
Elmer.out( beam.pos, beam.txt);
Elmer.run();
Elmer.up(beam.txt,-1,1,9Results/vmin,
         beam.txt,-1,2,9Results/vmax);

#OL.merge(RemoveViews.macro);

#4) Post-processing with Gmsh and a script
Post.register(interfaced);
Post.in(beam.pos, MNT.pos.ol, script.opt.ol);
Post.out(MNT.txt);
Post.run(beam.pos MNT.pos -);
Post.up(MNT.txt,-1,8,9Results/M,
        MNT.txt,-1,9,9Results/N,
        MNT.txt,-1,10,9Results/T);
Post.merge(RemoveViews.macro, beam.pos, script.opt);

#OL.msg(Loop index = <OL.get(X,attributes.get(Loop))> );
#OL.dump(aaa);