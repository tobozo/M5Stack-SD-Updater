#!/bin/bash


cd $PWD

#if ! [[ $TRAVIS_TAG ]]; then
#  git config --global user.email "travis@travis-ci.org"
#  git config --global user.name "Travis CI"
#  git tag ${TRAVIS_TAG}
#fi
#cd $REPO_NAME

pwd

cd $TRAVIS_BUILD_DIR
export git_version_last="$(git describe --abbrev=0 --tags --always)"
export git_version_next="v$(echo $git_version_last | awk -F . '{ printf "%d.%d.%d", $1,$2,$3 + 1}')"
#cd ..
echo $TRAVIS_BRANCH | grep "unstable" && export prerelease=true || export prerelease=false
git tag ${git_version_next}
echo "Before deploy Travis tag : $TRAVIS_TAG"
echo "Git version last         : $git_version_last"
echo "Git version next         : $git_version_next"
echo "Travis Branch            : $TRAVIS_BRANCH"
echo "Is pre-release           : $prerelease"
cd /home/travis/build/tobozo/

if [ -f $REPO_NAME/src/gitTagVersion.h ]; then
  echo "#define M5_SD_UPDATER_VERSION F(\"${TRAVIS_TAG}\")" > $REPO_NAME/src/gitTagVersion.h
else
  echo "Can't patch $REPO_NAME/src/gitTagVersion.h !!!"
  sleep 5
  exit 1
fi

zip -r $TRAVIS_BUILD_DIR/$REPO_NAME.zip $REPO_NAME -x *.git* -x "$REPO_NAME/examples/M5Stack-SD-Menu/SD-Apps" -x "$REPO_NAME/examples/M5Stack-SD-Menu/SD-Content"
if [ -f $TRAVIS_BUILD_DIR/$REPO_NAME.zip ]; then
  echo "Successfully Created $TRAVIS_BUILD_DIR/$REPO_NAME.zip :-)";
else
  echo "Failed to create $TRAVIS_BUILD_DIR/$REPO_NAME.zip !!!";
  sleep 5
  exit 1
fi

cd $M5_SD_BUILD_DIR
zip -r $TRAVIS_BUILD_DIR/$ARCHIVE_ZIP ./
if [ -f $TRAVIS_BUILD_DIR/$ARCHIVE_ZIP ]; then
  echo "Successfully created $TRAVIS_BUILD_DIR/$ARCHIVE_ZIP :-)";
else
  echo "Failed to create $TRAVIS_BUILD_DIR/$ARCHIVE_ZIP !!!";
  sleep 5
  exit 1
fi

cd $TRAVIS_BUILD_DIR



# export BODY=$(cat CHANGELOG.md) # boo! Travis doesn't like multiline body
