# Interactive 3D Airplane Carousel  (OpenGL Project)

This project is an **interactive 3D scene** implemented in C++ with **OpenGL (GLUT)**.  
It features hierarchical modeling, lighting, texture mapping, and multiple interaction modes.

- **User Interaction**  
  - **Keyboard**:  
    - `w/s/a/d` → look/rotate  
    - `q/e` → zoom in/out (third-person)  
    - `f` → switch to first-person  
    - `t` → switch to third-person  
    - `l` → toggle lighting  
    - `p` → change swing pattern  
    - `esc` → exit program  
  - **Right-click menu**: quick access to view, lighting, swing pattern, and quit.  

---

##  Demo Video
*(link to be added)*

---

##  Build Instructions
- **Dependencies**: FreeGLUT / OpenGL  
- **Compile** (Windows example with MinGW):
  ```bash
  g++ main.cpp -lfreeglut -lopengl32 -lglu32 -o Carousel.exe
