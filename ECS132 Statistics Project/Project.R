library(ggplot2)

#PROBLEM A

readData <- function()
{
	#Extracts the data from the file u.data with the following header
	colNames <- c("UserID", "MovieID", "Rating", "Timestamp")
	uData <- read.table("u.data", col.names = colNames)
	
	return(uData)
}

readUser <- function()
{
	#Extracts the data from the file u.user with the following header
	colNames <- c("UserID", "Age", "Gender", "Occupation", "ZipCode")
	uUser <- read.table("u.user", sep = "|", col.names = colNames)

	return(uUser)
}


mergeDataUser <- function()
{
	#Get the data frames for u.user and u.data
	data <- readData()
	user <- readUser()
	n <- intersect(names(data),names(user))
	#Merge them together where they intersect on headers
	merged <- merge(data, user, by=intersect(names(data), names(user)))
	write.table(merged, file = "u.merged", sep = "|", row.names = FALSE)
	return(merged)
}


getData <- function()
{
	#Get merged data to find the mean rating
	userRatings <- mergeDataUser()
	#Read in users to subset it for UserID and Gender
	users <- readUser()
	#Split the data from userRatings and find the mean of the ratings
	# per user
	ratings <- split(userRatings$Rating, userRatings$UserID)
	Mean <- sapply(ratings, mean)
	#Subset the user data frame to only UserID and Gender, and
	# append on the Mean ratings per UserID
	A <- subset(users, select=c("UserID", "Gender"))
	A$Mean <- c(Mean)

	return(A)
}

getDataNum <- function()
{
	#Get merged data for all ratings
	userRatings <- mergeDataUser()
	#Get user data
	users <- readUser()
	#Subset the data by UserID, Gender, and Ratings
	A <- subset(userRatings, select=c("UserID", "Gender", "Rating"))

	return(A)
}

confIntMen <- function()
{
	#Get the data for Mean User Ratings
	A <- getData()
	#Get a subset of only males
	men <- A[A$Gender=='M',]
	
	#Xbar - sample mean
	sampleMean <- mean(men$Mean)
	#n - sample population
	samplePop <- nrow(men)
	#s - standard deviation
	stdd <- sd(men$Mean)
	#1.96*s.e(theta) = the standard error applied to 95%
	# interval
	#Uses the equation s / sqrt(n)
	error <- qnorm(0.975)*stdd/sqrt(samplePop)
	#Find the left and right ends of the interval
	# Xbar -+ error
	left <- sampleMean - error
	right <- sampleMean + error

	cat("Sample Mean: ", sampleMean, "\n")
	cat("Sample Population: ", samplePop, "\n")
	cat("St Dev: ", stdd, " Error: ", error, "\n")
	cat("Interval: ", left, ", ", right, "\n")

	return(1)
}

confIntFemale <- function()
{
	#Get the data for Mean User Ratings
	A <- getData()
	#Get a subset of only females
	female <- A[A$Gender=='F',]

	#Xbar - sample mean
	sampleMean <- mean(female$Mean)
	#n - sample population
	samplePop <- nrow(female)
	#s - standard deviation
	stdd <- sd(female$Mean)
	#1.96*s.e(theta) = the standard error applied to 95%
	# interval
	#Uses the equation s / sqrt(n)
	error <- qnorm(0.975)*stdd/sqrt(samplePop)

	#Find the left and right ends of the interval
	left <- sampleMean - error
	right <- sampleMean + error

	cat("Sample Mean: ", sampleMean, "\n")
	cat("Sample Population: ", samplePop, "\n")
	cat("St Dev: ", stdd, " Error: ", error, "\n")
	cat("Interval: ", left, ", ", right, "\n")

	return(1)
}

confIntDiff <- function()
{
	#Get the data for Mean User Ratings
	A <- getData()
	#Get a subset of only males
	men <- A[A$Gender=='M',]
	#Get a subset of only females
	female <- A[A$Gender=='F',]

	#Xbar - sample mean for Males
	sampleMeanMen <- mean(men$Mean)
	#Ybar - sample mean for Females
	sampleMeanFemale <- mean(female$Mean)
	#Xbar - Ybar ; the difference of the sample means
	sampleMeanDiff <- abs(sampleMeanMen - sampleMeanFemale)
	#n_male - sample population of males
	samplePopM <- nrow(men)
	#n_female - sample population of females
	samplePopF <- nrow(female)
	#s_1 - standard deviation of male data
	stddM <- sd(men$Mean)
	#s_2 - standard deviation of female data
	stddF <- sd(female$Mean)
	#1.96*s.e(theta) = the standard error applied to 95%
	# interval
	#Uses the equation sqrt( ((s_1)^2 / n_male) + ((s_2)^2 / n_female) ) 
	error <- qnorm(0.975)* sqrt( (stddM^2 / samplePopM) + (stddF^2 / samplePopF) )
	#Apply the error to the intveral
	# Xbar - Ybar +- error
	left <- sampleMeanDiff - error
	right <- sampleMeanDiff + error

	cat("Sample Mean Male: ", sampleMeanMen, " Sample Mean Female: ", sampleMeanFemale, "\n")
	cat("Sample Population Male: ", samplePopM, " Sample Population Female: ", samplePopF, "\n")
	cat("St Dev Male: ", stddM, " St Dev Female: ", stddF, "\n")
	cat("Error: ", error, "\n")
	cat("Interval: ", left, ", ", right, "\n")

	return(error)
}

confIntPopRat <- function()
{
	#Get the full data to extra ratings
	A <- mergeDataUser()
	#Split ratings by UserID
	ratings <- split(A$Rating, A$UserID)
	#Find the number of ratings per user
	l <- sapply(ratings, length)
	#Get user data to parse out
	users <- readUser()
	#Parse the data by UserID and Gender
	TotRatPerUser <- subset(users, select=c("UserID", "Gender"))
	#Append on the number of ratings per userID
	TotRatPerUser$NumR <- c(l)
	#Get the full Data Set
	NonMeanA <- getDataNum()

	#Xbar - total number of male ratings
	m <- nrow(NonMeanA[NonMeanA$Gender == 'M',])
	#Ybar - total number of female ratings
	f <- nrow(NonMeanA[NonMeanA$Gender == 'F',])
	#Xbar - Ybar ; difference in female and male ratings
	sampleMeanDiff <- abs(m-f)
	#get the total number of ratings per User of males
	men <- TotRatPerUser[TotRatPerUser$Gender=='M',]
	#get the total number of ratings per User of females
	female <- TotRatPerUser[TotRatPerUser$Gender=='F',]
	#n_1 - sample population of men 
	samplePopM <- nrow(men)
	#n_2 - sample  population of female
	samplePopF <- nrow(female)
	#s_1 - standard deviation of the number of ratings by men
	stddM <- sd(men$NumR)
	#s_2 - standard deviation of the number of ratings be women
	stddF <- sd(female$NumR)

	#1.96*s.e(theta) = the standard error applied to 95%
	# interval
	#Uses the equation sqrt( ((s_1)^2 / n_1) + ((s_2)^2 / n_2) ) 
	error <- qnorm(0.975) * sqrt( (stddM^2 / samplePopM) + (stddF^2 / samplePopF) )
	#Apply the error to the intveral
	# Xbar - Ybar +- error
	left <- sampleMeanDiff - error
	right <- sampleMeanDiff + error

	cat("Sample Mean Male: ", m, " Sample Mean Female: ", f, "\n")
	cat("Sample Population Male: ", samplePopM, " Sample Population Female: ", samplePopF, "\n")
	cat("St Dev Male: ", stddM, " St Dev Female: ", stddF, "\n")
	cat("Error: ", error, "\n")
	cat("Interval: ", left, ", ", right, "\n")
	return(error)
}

confIntPropMale <- function()
{
	#Get the data mean ratings
	A <- getData()
	#Split the data into men and female frames
	men <- A[A$Gender=='M',]
	female <- A[A$Gender=='F',]
	#sample populations of men and women
	samplePopM <- nrow(men)
	samplePopF <- nrow(female)
	#n is the total sample population
	n <- samplePopM + samplePopF
	#p is the sample population of men divided by the total
	p <- samplePopM / n
	#will use 
	#1.96*s.e(theta) = the standard error applied to 95%
	# interval
	#Uses the equation sqrt( p*(1-p) / n ) where p = # men and n = totalPop
	error <- qnorm(0.975) * sqrt( ( p * (1 - p) ) / n)
	#Apply the error to the intveral
	# p +- error
	left <- p - error
	right <- p + error

	cat("Sample Mean: ", p, "\n")
	cat("Sample Population: ", n, "\n")
	cat("Error: ", error, "\n")
	cat("Interval: ", left, ", ", right, "\n")
}

hypothe <- function()
{
	#Get data of mean ratings
	A <- getData()
	#Seperate into male and female groups
	men <- A[A$Gender=='M',]
	female <- A[A$Gender=='F',]
	#mu0 - Hypothesis mean
	sampleMeanMen <- mean(men$Mean)
	#Xbar - True mean
	sampleMeanFemale <- mean(female$Mean)
	xbar <- sampleMeanFemale
	mu0 <- sampleMeanMen
	#sigma - standard deviation of ratings of men
	sigma <- sd(men$Mean)
	#Number of men
	n <- nrow(men)
	#Uses equation 11.6
	z <- (xbar - mu0)/(sigma/sqrt(n))

	return(z)
}


histo <- function()
{
	#Get the data of mean ratings
	A <- getData()
	#Split into female and male subsets
	men <- A[A$Gender=='M',]
	female <- A[A$Gender=='F',]

	#Produce histograms based off the average ratings per females and males
	malePlot <- ggplot() + aes(men$Mean) + geom_histogram(binwidth = 0.5, colour =  "black", fill = "dodgerblue3") + labs(title = "Mean Male Movie Ratings", x = "Mean Ratings", y = "Frequency")
	femalePlot <- ggplot() + aes(female$Mean) + geom_histogram(binwidth = 0.5, colour =  "black", fill = "indianred3") + labs(title = "Mean Female Movie Ratings", x = "Mean Ratings", y = "Frequency")
	#Save to the respective files maleHistogram.png and femaleHistrogram.png
	ggsave(malePlot, file="maleHistogram.png")
	ggsave(femalePlot, file="femaleHistogram.png")
}

linearMod <- function()
{
	#Get the raw data of the merged Users and Data
	userRatings <- mergeDataUser()
	#Get the raw data of users because we need age
	users <- readUser()
	#Split the ratings by UserID
	ratings <- split(userRatings$Rating, userRatings$UserID)	
	#Create the data frame A consisting of UserID, Gender, and Age
	A <- subset(users, select=c("UserID", "Gender", "Age"))
	#Find the mean rating per User
	Mean <- sapply(ratings, mean)
	#Append the last value onto our data frame
	A$Mean <- c(Mean)
	#Convert gender to binary 0 and 1 indicator variables
	A$Gen <- as.numeric(A$Gender == 'M')

	#Data frame B is used for A.h, finding the mean average ratings for females
	# of age 28
	B <- A[A$Gender=='F',]
	B <- B[B$Age==28,]

	#Call lm (linear model) to estimate mean ratings from gender and age
	sum <- summary(lm(A$Mean ~ A$Gen + A$Age))
	#Call lm (linear model) to estimate mean ratings for women of age 28
	womanAge <- summary(lm(B$Mean ~ B$Gen + B$Age))
	#Save both to a file for easy copy-pasting
	capture.output(sum, file = "reg.data")
	capture.output(womanAge, file = "woman.dat")

	#Plot the scatter plot and linear model for estimating
	# mean ratings from gender and age using GGplot
	plot <- ggplot() + aes(x=A$Gen+A$Age, y=A$Mean) + geom_point(shape = 1) + geom_smooth(method=lm, se=TRUE) + labs(title = "Linear Model for Rating", y = "Mean Estimated Rating", x = "Age + Gender")
	ggsave(plot, file="regression.png")

	return(1)
}

#PROBLEM B
readDataB <- function()
    {
      bank <- read.csv(file="vocabulary_norms_data.csv", head=TRUE, sep=",")[,
            c('age', 'birth_order', 'ethnicity', 'sex', 'mom_ed', 'vocab')]
      clean <- bank[complete.cases(bank), ]
      return(clean)
    }

AgeVocab <-function()
{
  dataB <- readDataB()
  #Mean, standard deviation and sample size for age and vocabulary
  mean_age <- mean(dataB$age)
  mean_vocab <- mean(dataB$vocab)
  cat("Average age", mean_age, "\n")
  cat("Average vocabulary", mean_vocab, "\n")
  sd_age <- sd(dataB$age)
  sd_vocab <- sd(dataB$vocab)
  cat("Standard deviation: age", sd_age, "\n")
  cat("Standard deviation: vocabulary", sd_vocab, "\n")
  size_age <- length(dataB$age)
  size_vocab <- length(dataB$vocab)
  cat("Sample size: age", size_age, "\n")
  cat("Sample size: vocabulary", size_vocab, "\n")
  # COnfidence interval for age
  error_age  <- qnorm(0.975)*sd_age/sqrt(size_age) 
  left_age <- mean_age - error_age
  right_age<- mean_age + error_age
  cat("Standard error: age", error_age, "\n")
  cat("95% Confidence Interval", left_age, " , ", right_age, "\n")
  # Confidence interval for vocabulary    
  error_vocab  <- qnorm(0.975)*sd_vocab/sqrt(size_vocab)
  left_vocab <- mean_vocab - error_vocab
  right_vocab<- mean_vocab + error_vocab
  cat("Standard error: vocabulary", error_vocab, "\n")
  cat("95% Confidence Interval", left_vocab, " , ", right_vocab, "\n")

  #linear model to find correlation between age and vocabulary
  sum <- summary(lm(dataB$age ~ dataB$vocab))
  capture.output(sum, file = "age_vocab.data")
  AgeVocab <- ggplot() + aes(x = dataB$age, y = dataB$vocab)+ geom_point(shape = 1) + geom_smooth(method = lm, se=TRUE) + labs(title = "Linear Model for  Age and Vocabulary", x = "Age", y = "Vocabulary")
  ggsave(AgeVocab, file="AgeVocab.png")
}

BirthOrderVocab <- function()
{
dataB <- readDataB()
m <- length(dataB$age)
#extract unique values
n <- length(unique(dataB$birth_order))
orders <- (unique(dataB$birth_order))
birth <- as.vector(orders)
#cat("orders" , orders)
means <- vector( , n)
sds <- vector( ,n )
error <- vector( ,n)
ls <- vector( ,n)
rs <- vector( ,n)
sizes <- vector( ,n)
for(i in 1:n)
#first <- dataB[dataB$birth_order == 'First', ]
{
order <- dataB[(dataB$birth_order == birth[i]), ]
cat("Birth order: ",  birth[i], "\n")
if(length(order$vocab) > 1)
{
means[i] <- mean(order$vocab)
cat("Mean: ", means[i], "\n")
sds[i] <- sd(order$vocab)
cat("Standard Deviation: ", sds[i], "\n")
sizes[i] <- length(order$vocab)
cat("Sample Size:", sizes[i], "\n")
error[i] <-  qnorm(0.975)*sds[i]/sqrt(sizes[i])
cat("Standard error: ", error[i], "\n")
ls[i] <- means[i] - error[i]
rs[i] <- means[i] + error[i]
cat("95% Confidence Interval: ", ls[i], " , ", rs[i], "\n")
}
else
{
means[i] <- order$vocab
sizes[i] <-1
sds[i] <- 0
error[i] <- 0
ls[i] <- means[i]
rs[i] <- means[i]
cat("Mean: ", means[i], "\n")
cat("Standard Deviation: ", sds[i], "\n")
cat("Sample Size:", sizes[i], "\n")
cat("Standard error: ", error[i], "\n")
cat("95% Confidence Interval: ", ls[i], " , ", rs[i], "\n")
}
}
births<- vector( , n)
for(j in 1:n)
{
if(birth[j] == 'First')
 births[j] = 1
if(birth[j] == 'Second')
 births[j] = 2
if(birth[j] == 'Third')
 births[j] = 3 
if(birth[j] == 'Fourth')
 births[j] = 4
if(birth[j] == 'Fifth')
 births[j] = 5
if(birth[j] == 'Sixth')
 births[j] = 6
if(birth[j] == 'Seventh')
 births[j] = 7
if(birth[j] == 'Eighth')
 births[j] = 8
}
sum <- summary(lm(births ~ means))
capture.output(sum, file = "birth_mean_vocab.data")

means_order <- ggplot() + aes(x = births, y = means)+ geom_point(shape = 1) + geom_smooth(method = lm, se=TRUE) + labs(title = "Linear Model for Birth Order and Average Vocabulary", x = "Birth Order as Integer", y = "Average Vocabulary")
ggsave(means_order, file="means_orders.png")

}

