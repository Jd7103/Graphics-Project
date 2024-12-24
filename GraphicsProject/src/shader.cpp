#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;
    std::ifstream geometryShaderFile;
    
    vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
      
        vertexShaderFile.open(vertexPath);
        fragmentShaderFile.open(fragmentPath);
        std::stringstream vertexShaderStream, fragmentShaderStream;
     
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();
      
        vertexShaderFile.close();
        fragmentShaderFile.close();
       
        vertexCode = vertexShaderStream.str();
        fragmentCode = fragmentShaderStream.str();
        
        if (geometryPath != nullptr)
        {
            geometryShaderFile.open(geometryPath);
            std::stringstream geometryShaderStream;
            geometryShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = geometryShaderStream.str();
        }
    }
    catch (std::ifstream::failure& e) {

        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;

    }

    const char* vertexShaderCode = vertexCode.c_str();
    const char* fragmentShaderCode = fragmentCode.c_str();
    
    GLuint vertex, fragment;
    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    
    GLuint geometry;
    if (geometryPath != nullptr)
    {
        const char* geometryShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometryShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
    
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr)
        glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (geometryPath != nullptr)
        glDeleteShader(geometry);

}

void Shader::use() 
{
    glUseProgram(ID);

}

void Shader::setBool(const std::string& name, bool value) const {

    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);

}

void Shader::setInt(const std::string& name, int value) const {

    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);

}

void Shader::setFloat(const std::string& name, float value) const {

    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);

}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
     
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);

}

void Shader::setVec2(const std::string& name, float x, float y) const {

    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);

}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {

    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);

}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {

    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);

}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {

    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);

}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {

    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);

}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const {

    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);

}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {

    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);

}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {

    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);

}

void Shader::checkCompileErrors(GLuint shader, std::string type) {

    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {

            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        
        }
    }
    else {

        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {

            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        
        }
    }
}