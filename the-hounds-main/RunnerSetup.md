# GitLab CI/CD Pipeline Setup Manual

This manual will guide you through setting up a complete CI/CD pipeline in GitLab for a C++ project with automated testing, coverage reporting, and documentation generation.

## Overview

The pipeline consists of 4 stages:
1. **Lint** - Code quality checks using clang-format
2. **Build** - Compilation, testing, and coverage analysis
3. **Docs** - Documentation generation with Doxygen
4. **Deploy** - Publishing artifacts to a separate branch

## Prerequisites

Before setting up the pipeline, ensure your project has:
- C++ source code in `src/` directory
- Test files in `tests/` directory
- A `CMakeLists.txt` file with coverage support
- A `Doxyfile` for documentation generation

## Step 1: Create the Pipeline Configuration

1. In your GitLab project, create a file named `.gitlab-ci.yml` in the root directory
2. Copy the pipeline configuration provided (the complete .gitlab-ci.yml content)
3. Commit and push this file to your repository

## Step 2: Configure Project Settings

### Enable GitLab General pipelines
1. Go to **Settings** → **General**
2. Open the menu for **Visibility, project features, permission**
3. Under Repository activate the toggle for **CI/CD**
4. Scroll down and click **Save changes**

Now CI/CD pipelines are activated in your repository. You now need to configure it.

### Set up Project Access Token
1. Go to **Settings** → **Access Tokens**
2. Click **Add new token**
3. Configure the token:
   - **Token name**: `CI_BADGE_PUBLISHER`
   - **Expiration date**: Set appropriate expiration (e.g., 1 year)
   - **Select a role**: Set this to *Maintainer*
   - **Select scopes**: Check `write_repository`, `read_repository`
4. Click **Create project access token**
5. **Important**: Copy the token value immediately (you won't see it again)

### Add CI/CD Variables
1. Navigate to **Settings** → **CI/CD**
2. Expand the **Variables** section
3. Click **Add variable**
4. Add the following variable:
   - **Key**: `GITLAB_PUSH_TO_SITE_TOKEN`
   - **Value**: Paste the token you copied above
   - **Type**: Variable
   - **Environment scope**: All
   - **Protect variable**: unchecked
   - **Mask variable**: checked
   - **Expand variable reference**: checked
5. Click **Add variable**

## Step 3: Repository Structure Requirements

Ensure your project has the following structure:
```
project-root/
├── .gitlab-ci.yml          # Pipeline configuration
├── CMakeLists.txt          # Build configuration with coverage support
├── Doxyfile                # Doxygen configuration
├── src/                    # Source code directory
│   ├── *.cpp
│   └── *.hpp
└── tests/                  # Test directory
    ├── *.cpp
    └── *.hpp
```

## Step 4: CMakeLists.txt Configuration

Your `CMakeLists.txt` should support coverage reporting. Ensure it includes:

```cmake
# Enable coverage option
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
```

## Step 5: Test the Pipeline

1. Make any small change to your code
2. Commit and push to the `master` branch
3. Go to **CI/CD** → **Pipelines** in your GitLab project
4. Watch the pipeline execution

### Pipeline Stages Explained

#### Lint Stage
- Runs `clang-format` to check code formatting
- Uses Google style guide
- Will fail if code doesn't meet formatting standards

#### Build Stage
- Compiles your C++ project using CMake
- Runs all tests and generates JUnit XML reports
- Creates coverage reports (XML and HTML)
- Generates testing and coverage badges
- Artifacts include test results, coverage reports, and badges

#### Docs Stage
- Generates documentation using Doxygen
- Creates HTML documentation in `docs/html/`

#### Deploy Stage
- Publishes badges and documentation to a `site` branch
- Only runs on the `master` branch
- Makes documentation available via GitLab Pages

## Step 6: View Results

### Test Results
- Go to **CI/CD** → **Pipelines** → Click on a pipeline
- Click on the **build-job** stage
- View the **Tests** tab for test results

### Coverage Reports
- In the pipeline view, click on **build-job**
- Download artifacts to view detailed HTML coverage reports
- Coverage percentage is displayed in the job output

### Documentation
- After successful deployment, documentation will be available at:
  `https://gitlab.inf.ethz.ch/course-secse-2025/[group-name]/-/site/docs/html`

### Badges
- Testing and coverage badges are automatically generated
- They will be available in the `site` branch
- Can be embedded in your README.md:
```markdown
![Tests](https://gitlab.inf.ethz.ch/[group-name]/-/raw/site/testing-badge.svg)
![Coverage](https://gitlab.inf.ethz.ch/[group-name]/-/raw/site/coverage-badge.svg)
```

## Troubleshooting

### Common Issues

1. **Pipeline fails on lint stage**
   - Run `clang-format -i -style=google $(find src tests -name "*.cpp" -o -name "*.hpp")` locally
   - Commit the formatted code

2. **Build stage fails**
   - Check that your CMakeLists.txt is properly configured
   - Ensure all dependencies are available in Ubuntu 24.04

3. **Deploy stage fails**
   - Verify that `GITLAB_PUSH_TO_SITE_TOKEN` is correctly set
   - Check that the token has `write_repository` permissions
   - Maybe you might need to first create a protected site branch.

4. **Coverage not showing**
   - Ensure your tests actually exercise the code
   - Verify that `ENABLE_COVERAGE=ON` is set in CMake configuration

### Viewing Detailed Logs
1. Go to **CI/CD** → **Pipelines**
2. Click on the failed pipeline
3. Click on the failed job
4. View the complete log output for debugging

## Customization Options

### Modify Coverage Thresholds
In `.gitlab-ci.yml`, you can adjust the coverage badge colors by modifying the percentages in the `.create_coverage_badge` section.


## Best Practices

1. **Keep your master branch stable** - The deploy stage only runs on master
2. **Write comprehensive tests** - Better test coverage leads to more reliable software
3. **Follow consistent formatting** - Use the lint stage to maintain code quality
4. **Review pipeline results regularly** - Monitor test failures and coverage trends
5. **Update documentation** - Keep your Doxyfile and code comments current
