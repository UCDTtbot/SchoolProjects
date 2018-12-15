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

ethnicity_vocab <- function()
{
dataB <- readDataB()
#extract unique values
n <- length(unique(dataB$ethnicity))
eth <- (unique(dataB$ethnicity))
ethn <- as.vector(eth)
#cat("orders" , orders)
means <- vector( , n)
sds <- vector( ,n )
error <- vector( ,n)
ls <- vector( ,n)
rs <- vector( ,n)
sizes <- vector( ,n)
for(i in 1:n)
{
order <- dataB[(dataB$ethnicity == ethn[i]), ]
cat("Ethnicity: ",  ethn[i], "\n")
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
int_eth <- vector( ,n)
for(j in 1:n)
{
if(ethn[j] == 'Asian')
 int_eth[j] = 1
if(ethn[j] == 'Black')
 int_eth[j] = 2
if(ethn[j] == 'Other')
 int_eth[j] = 3
if(ethn[j] == 'White')
 int_eth[j] = 4
if(ethn[j] == 'Hispanic')
 int_eth[j] = 5
}
sum <- summary(lm(int_eth ~ means))
capture.output(sum, file = "ethnicity_mean_vocab.data")

means_order <- ggplot() + aes(x =int_eth, y = means)+ geom_point(shape = 1) + geom_smooth(method = lm, se=TRUE) + labs(title = "Linear Model for Ethnicity and Average Vocabulary", x = "Ethnicity as Integer", y = "Average Vocabulary")
ggsave(means_order, file="means_ethn.png")

}
}


education <- function()
{
dataB <- readDataB()
#extract unique values
n <- length(unique(dataB$mom_ed))
ed <- (unique(dataB$mom_ed))
edu <- as.vector(ed)
#cat("orders" , orders)
means <- vector( , n)
sds <- vector( ,n )
error <- vector( ,n)
ls <- vector( ,n)
rs <- vector( ,n)
sizes <- vector( ,n)
for(i in 1:n)
{
order <- dataB[(dataB$mom_ed == edu[i]), ]
cat("Eductation: ", edu[i], "\n")
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
int_edu <- vector( , n)
for(j in 1:n)
{
if(edu[j] == "Graduate")
int_edu[j] = 1
if(edu[j] == "College")
int_edu[j] = 2
if(edu[j] == "Some Secondary")
int_edu[j] = 3
if(edu[j] == "Secondary")
int_edu[j] = 4
if(edu[j] == "Some College")
int_edu[j] = 5
if(edu[j] == "Primary")
int_edu[j] = 6
if(edu[j] == "Some Graduate")
int_edu[j] = 7
}
sum <- summary(lm(int_edu ~ means))
capture.output(sum, file = "edu_mean_vocab.data")

means_order <- ggplot() + aes(x =int_edu, y = means)+ geom_point(shape = 1) + geom_smooth(method = lm, se=TRUE) + labs(title = "Linear Model for Mother's Education and Average Vocabulary", x = "Mother's Education as Integer", y = "Average Vocabulary")
ggsave(means_order, file="means_edu.png")

}




gender_vocab <- function()
{
dataB <- readDataB()
#extract unique values
n <- length(unique(dataB$sex))
sex <- (unique(dataB$sex))
gender <- as.vector(sex)
#cat("orders" , orders)
means <- vector( , n)
sds <- vector( ,n )
error <- vector( ,n)
ls <- vector( ,n)
rs <- vector( ,n)
sizes <- vector( ,n)
for(i in 1:n)
{
order <- dataB[(dataB$sex == gender[i]), ]
cat("Sex: ",  gender[i], "\n")
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

int_sex <- vector( , n)
for(j in 1:n)
{
if(gender[j] == 'Male')
 int_sex[j] = 1
if(gender[j] == 'Female')
 int_sex[j] = 2
}

sum <- summary(lm(int_sex ~ means))
capture.output(sum, file = "sex_mean_vocab.data")

#means_order <- ggplot() + aes(x =int_sex, y = means)+ geom_point(shape = .5) + geom_smooth(method = lm, se=TRUE) + labs(title = "Linear Model for Sex and Average Vocabulary", x = "Sex as Integer", y = "Average Vocabulary")
#ggsave(means_order, file="means_sex.png")
}








