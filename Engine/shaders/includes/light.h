
mat4 getLightViewMatrix() {
    vec3 lightPos = vec3(3.0, 5.0, -2.0);
    vec3 target   = vec3(0.0, 0.0, 0.0);
    vec3 up       = vec3(0.0, 1.0, 0.0);

    vec3 f = normalize(target - lightPos); // Forward vector
    vec3 r = normalize(cross(f, up));        // Right vector
    vec3 u = cross(r, f);                    // Recomputed Up vector

    // Constructing a lookAt matrix (column-major order)
    return mat4(
        vec4(r, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(-dot(r, lightPos), -dot(u, lightPos), dot(f, lightPos), 1.0)
    );
}

mat4 getLightOrthoMatrix() {
    float left   = -10.0;
    float right  =  10.0;
    float bottom = -10.0;
    float top    =  10.0;
    float zNear  =  1.0;
    float zFar   =  7.5;

    // Constructing an orthographic projection matrix
    return mat4(
        vec4(2.0 / (right - left),         0.0,                         0.0, 0.0),
        vec4(0.0,                          2.0 / (top - bottom),         0.0, 0.0),
        vec4(0.0,                          0.0,                       -2.0 / (zFar - zNear), 0.0),
        vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1.0)
    );
}

