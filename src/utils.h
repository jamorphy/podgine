char* read_shader_file(const char* filepath) {
    printf("Attempting to load shader: %s\n", filepath);
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open shader file: %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        printf("Failed to allocate memory for shader\n");
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}
