#!/bin/bash


cd $PWD
  
#if ! [[ $TRAVIS_TAG ]]; then
#  git config --global user.email "travis@travis-ci.org"
#  git config --global user.name "Travis CI"
#  git tag ${TRAVIS_TAG}
#fi
cd $REPO_NAME
export git_version_last="$(git describe --abbrev=0 --tags --always)" 
export TRAVIS_TAG="$(echo $git_version_last | awk -F . '{ printf "%d.%d.%d", $1,$2,$3 + 1}')"
cd ..
echo $TRAVIS_BRANCH | grep "unstable" && export prerelease=true || export prerelease=false
git tag ${TRAVIS_TAG}
echo "Before deploy Travis tag : $TRAVIS_TAG"
echo "Travis Branch            : $TRAVIS_BRANCH"
echo "Is pre-release           : $prerelease"
cd /home/travis/build/tobozo/
echo "#define M5_SD_UPDATER_VERSION F(\"${TRAVIS_TAG}\")" > $REPO_NAME/src/gitTagVersion.h
zip -r $TRAVIS_BUILD_DIR/$REPO_NAME.zip $REPO_NAME -x *.git* -x "$REPO_NAME/examples/M5Stack-SD-Menu/SD-Apps" -x "$REPO_NAME/examples/M5Stack-SD-Menu/SD-Content"
cd $M5_SD_BUILD_DIR 
zip -r $TRAVIS_BUILD_DIR/$ARCHIVE_ZIP ./
cd $TRAVIS_BUILD_DIR

# export BODY=$(cat CHANGELOG.md) # boo! Travis doesn't like multiline body
