all:
	g++ main.cpp PlaneMeshGenerator.cpp BezierCurveGenerator.cpp -o main -g -lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL -I.
