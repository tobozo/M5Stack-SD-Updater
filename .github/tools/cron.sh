#!/bin/bash

# set -o xtrace

git remote rm origin
git remote add origin https://$GITHUB_ACTOR:$GITHUB_TOKEN@github.com/$GITHUB_REPOSITORY.git
git config --global user.name $GITHUB_ACTOR
git config --global user.email $GITHUB_ACTOR@users.noreply.github.com

nextAW=`curl -L --silent $WORKFLOW_URL | jq -r .need_workflow[0]`
if [[ "$nextAW" == "" || "$nextAW" == "null" ]]; then
  echo "No workflow in queue"
  exit 0;
fi
nextURL=`echo "$nextAW" | jq  -r .workflow_url`;
if [[ "$nextURL" == "" || "$nextURL" == "null" ]]; then
  echo "No workflow_url in first queued item, something wrong on remote? JSON chunk: $nextAW"
  exit 0;
fi

workflowFilename="${nextURL##*/}"

cd .github/workflows

if [ -f $workflowFilename ]; then
  echo "Deleting $workflowFilename"
  git rm --cached $workflowFilename
  git commit -m "Auto-removed $workflowFilename from cron workflow"
fi

wget "$nextURL" --output-document=$workflowFilename
touch $workflowFilename
git add $workflowFilename
# TODO: also remove unwanted/obsoleted workflow files
git commit -m "Auto-committed from cron workflow"
git push -u origin master

# now generate a comment if necessary
comments_url=`echo "$nextAW" | jq  -r .comments_url`;
if [[ "$comments_url" == "" || "$comments_url" == "null" ]]; then
  echo "This workflow has no comments_url"
  exit 0;
fi

# and post the comment
commentMessage='The app has been scheduled for rebuild'

# errorless jq call
allComments=`curl -L --silent $comments_url | jq -r ".[]|. as \\$in|try (try .body catch error( {\"error\": \\$in })) catch {}"`
# echo "Captured comments: $allComments"

# avoid posting dupe comment
if [[ $allComments == *"$commentMessage"* ]]; then
  echo "Rebuild comment already posted"
else
  jsonData='{"body":"The app has been scheduled for rebuild"}'
  curlAuth="Authorization: token $GITHUB_TOKEN"
  HTTP_RESPONSE=$(curl -X POST -H "$curlAuth" -H "Content-Type: application/json" --data "$jsonData" -L --silent --write-out "HTTPSTATUS:%{http_code}" $comments_url)
  HTTP_BODY=$(echo $HTTP_RESPONSE | sed -e 's/HTTPSTATUS\:.*//g')
  HTTP_STATUS=$(echo $HTTP_RESPONSE | tr -d '\n' | sed -e 's/.*HTTPSTATUS://')
  if [[ "$HTTP_STATUS" == "201" ]]; then
    echo " ---> GitHub server HTTP response to POST Comment: $HTTP_STATUS"
  else
    echo " ---> Comment post failed, HTTP_RESPONSE : $HTTP_RESPONSE"
  fi
fi
