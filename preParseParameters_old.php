<?php

function generateHeaderFile($jsonFile, $namespace) {
    $jsonData = file_get_contents($jsonFile);
    $data = json_decode($jsonData, true);

    if (!$data) {
        echo "Failed to parse JSON from $jsonFile\n";
        return;
    }

    $headerFile = dirname($jsonFile) . '/parameters.h';
    $file = fopen($headerFile, "w");

    // Write the namespace declaration
    fwrite($file, "#pragma once\n");
    fwrite($file, "namespace " . $namespace . " {\n\n");

    fwrite($file, "#include <unordered_map>\n");
    fwrite($file, "#include <string>\n");
    fwrite($file, "#include <vector>\n\n");

    // Write the ParamID enum
    fwrite($file, "enum class ParamID {\n");
    foreach ($data as $paramName => $paramInfo) {
        fwrite($file, "    " . ucfirst($paramName) . ",\n");
    }
    fwrite($file, "    Unknown\n};\n\n");

    // Write the parameter type enum
    fwrite($file, "enum class ParamType {\n");
    fwrite($file, "    Dec,\n");
    fwrite($file, "    Opt,\n");
    fwrite($file, "    Unknown\n};\n\n");

    // Write the parameter options enums
    $optionEnums = [];
    foreach ($data as $paramName => $paramInfo) {
        if (isset($paramInfo['options']) && $paramInfo['type'] === 'opt') {
            $options = explode(',', $paramInfo['options']);
            $enumName = ucfirst($paramName) . "Options";
            fwrite($file, "enum class " . $enumName . " {\n");
            foreach ($options as $option) {
                fwrite($file, "    " . trim($option) . ",\n");
            }
            fwrite($file, "    Unknown\n};\n\n");
            $optionEnums[$paramName] = $enumName;
        }
    }

    // Write the string-to-enum map
    fwrite($file, "const std::unordered_map<std::string, ParamID> paramMap = {\n");
    foreach ($data as $paramName => $paramInfo) {
        fwrite($file, '    {"' . $paramName . '", ParamID::' . ucfirst($paramName) . "},\n");
    }
    fwrite($file, "};\n");

    // Write the default values map
    fwrite($file, "\nconst std::unordered_map<ParamID, std::pair<ParamType, double>> defaultValues = {\n");
    foreach ($data as $paramName => $paramInfo) {
        $default = isset($paramInfo['default']) ? $paramInfo['default'] : 0;
        $type = $paramInfo['type'] === 'dec' ? "ParamType::Dec" : "ParamType::Opt";
        fwrite($file, '    {ParamID::' . ucfirst($paramName) . ', {' . $type . ', ' . $default . "}},\n");
    }
    fwrite($file, "};\n");

    // Write the option mappings
    foreach ($optionEnums as $paramName => $enumName) {
        fwrite($file, "\nconst std::unordered_map<int, " . $enumName . "> " . ucfirst($paramName) . "OptionsMap = {\n");
        $options = explode(',', $data[$paramName]['options']);
        foreach ($options as $index => $option) {
            fwrite($file, '    {' . $index . ', ' . $enumName . "::" . trim($option) . "},\n");
        }
        fwrite($file, "};\n");
    }

    // End the namespace declaration
    fwrite($file, "\n} // namespace " . $namespace . "\n");

    fclose($file);
    echo "Generated $headerFile\n";
}

// Recursive function to traverse directories
function traverseDirectories($dir, $namespace = '') {
    $files = glob($dir . '/*', GLOB_MARK);
    foreach ($files as $file) {
        if (is_dir($file)) {
            $currentNamespace = $namespace . (!empty($namespace) ? "::" : "") . basename($file);
            traverseDirectories($file, $currentNamespace);
        } elseif (basename($file) === 'parameters.json') {
            generateHeaderFile($file, $namespace);
        }
    }
}

// Start traversal from the current directory
traverseDirectories(__DIR__);
